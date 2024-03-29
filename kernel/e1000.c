#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "e1000_dev.h"
#include "net.h"

#define TX_RING_SIZE 16
static struct tx_desc tx_ring[TX_RING_SIZE] __attribute__((aligned(16)));
static struct mbuf *tx_mbufs[TX_RING_SIZE];

#define RX_RING_SIZE 16
static struct rx_desc rx_ring[RX_RING_SIZE] __attribute__((aligned(16))); // the descriptor format receive ring/queue
static struct mbuf *rx_mbufs[RX_RING_SIZE]; // mbuf packet buffers for the e1000

// remember where the e1000's registers live.
static volatile uint32 *regs; // 保存了第一个控制寄存器

struct spinlock e1000_lock;
struct spinlock e1000_txlock;


// called by pci_init().
// xregs is the memory address at which the
// e1000's registers are mapped. 
// configure the E1000
void
e1000_init(uint32 *xregs)
{
  int i;

  initlock(&e1000_lock, "e1000");
  initlock(&e1000_txlock, "e1000");

  regs = xregs;

  // Reset the device
  regs[E1000_IMS] = 0; // disable interrupts
  regs[E1000_CTL] |= E1000_CTL_RST;
  regs[E1000_IMS] = 0; // redisable interrupts
  __sync_synchronize();

  // [E1000 14.5] Transmit initialization transmit
  memset(tx_ring, 0, sizeof(tx_ring));
  for (i = 0; i < TX_RING_SIZE; i++) {
    tx_ring[i].status = E1000_TXD_STAT_DD;
    tx_mbufs[i] = 0;
  }
  regs[E1000_TDBAL] = (uint64) tx_ring;
  if(sizeof(tx_ring) % 128 != 0)
    panic("e1000");
  regs[E1000_TDLEN] = sizeof(tx_ring);
  regs[E1000_TDH] = regs[E1000_TDT] = 0;
  
  // [E1000 14.4] Receive initialization receive
  memset(rx_ring, 0, sizeof(rx_ring));
  for (i = 0; i < RX_RING_SIZE; i++) {
    rx_mbufs[i] = mbufalloc(0); // allocate mbuf
    if (!rx_mbufs[i])
      panic("e1000");
    rx_ring[i].addr = (uint64) rx_mbufs[i]->head;
  }
  regs[E1000_RDBAL] = (uint64) rx_ring;
  if(sizeof(rx_ring) % 128 != 0)
    panic("e1000");
  regs[E1000_RDH] = 0;
  regs[E1000_RDT] = RX_RING_SIZE - 1;
  regs[E1000_RDLEN] = sizeof(rx_ring);

  // filter by qemu's MAC address, 52:54:00:12:34:56
  regs[E1000_RA] = 0x12005452;
  regs[E1000_RA+1] = 0x5634 | (1<<31);
  // multicast table
  for (int i = 0; i < 4096/32; i++)
    regs[E1000_MTA + i] = 0;

  // transmitter control bits.
  regs[E1000_TCTL] = E1000_TCTL_EN |  // enable
    E1000_TCTL_PSP |                  // pad short packets
    (0x10 << E1000_TCTL_CT_SHIFT) |   // collision stuff
    (0x40 << E1000_TCTL_COLD_SHIFT);
  regs[E1000_TIPG] = 10 | (8<<10) | (6<<20); // inter-pkt gap

  // receiver control bits.
  regs[E1000_RCTL] = E1000_RCTL_EN | // enable receiver
    E1000_RCTL_BAM |                 // enable broadcast
    E1000_RCTL_SZ_2048 |             // 2048-byte rx buffers
    E1000_RCTL_SECRC;                // strip CRC
  
  // ask e1000 for receive interrupts.
  regs[E1000_RDTR] = 0; // interrupt after every received packet (no timer)
  regs[E1000_RADV] = 0; // interrupt after every packet (no timer)
  regs[E1000_IMS] = (1 << 7); // RXDW -- Receiver Descriptor Write Back
}
int
e1000_transmit(struct mbuf *m)
{
  //
  // Your code here.
  //
  // the mbuf contains an ethernet frame; program it into
  // the TX descriptor ring so that the e1000 sends it. Stash
  // a pointer so that it can be freed after sending.
  //
  // printf("transmit start!\n");
  // ask for Tx ring index by reading he E1000_TDT control reg
// must lock!!!!!!!!
  acquire(&e1000_txlock);
  int tail = regs[E1000_TDT],head = regs[E1000_TDH];
  // printf("head:%d tail:%d\n",head,tail,regs[E1000_TDLEN]);
  // then check if the ring is overflowing
  if ((tail + 1) % TX_RING_SIZE == head){
    printf("tx overflow!\n");
    return -1;
  }
  // E10000_TXDSTAT_DD finished
  if ((tx_ring[tail].status & E1000_TXD_STAT_DD) == 0){
    printf("not finshed!\n");
    return -1;
  }
  if (tx_mbufs[tail])
    mbuffree(tx_mbufs[tail]);
  // fill descrpitor
  tx_ring[tail].addr = (uint64)m->head;
  tx_ring[tail].length = (uint16)m->len;
  tx_ring[tail].cmd = 0x01;
  tx_mbufs[tail] = m;
  // update ring position
  regs[E1000_TDT] = (tail + 1) % TX_RING_SIZE;
  release(&e1000_txlock);
  return 0;
}

static void
e1000_recv(void)
{
  //
  // Your code here.
  //
  // Check for packets that have arrived from the e1000
  // Create and deliver an mbuf for each packet (using net_rx()).
  // ask for index and update it
  // printf("receive start!\n");
  while(1)
  {
    int tail = (regs[E1000_RDT] + 1) % RX_RING_SIZE;
    // printf("recv tail:%d head:%d \n",tail - 1,regs[E1000_RDH]);
    // check new packet is available
    if (rx_ring[tail].status  == 0){
      // printf("not ready rxd!\n");
      return ;
    }
    // if (tail == head)
    // update len and deliver mbuf by calling net_rx
    rx_mbufs[tail]->len = rx_ring[tail].length;
    // printf("rx_mbufs[tail]:%d %d %p\n",tail,rx_mbufs[tail]->len,*rx_mbufs[tail]->head);
    net_rx(rx_mbufs[tail]);
    
    // allocate a new mbuf and set head and clear status to zero
    rx_mbufs[tail] = mbufalloc(0);
    if (!rx_mbufs[tail])
      panic("e1000");
    rx_ring[tail].addr = (uint64)rx_mbufs[tail]->head; 
    rx_ring[tail].status = 0;
    // update rdt
    regs[E1000_RDT] = tail;
  }
}

void
e1000_intr(void)
{
  // tell the e1000 we've seen this interrupt;
  // without this the e1000 won't raise any
  // further interrupts.
  // acquire(&e1000_lock);
  regs[E1000_ICR] = 0xffffffff;
  e1000_recv();
  // release(&e1000_lock);
}
