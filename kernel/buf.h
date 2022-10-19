struct buf {
  int valid;   // has data been read from disk?
  int disk;    // does disk "own" buf?
  uint dev;
  uint blockno; // seem like tu be the number of block
  struct sleeplock lock;
  uint refcnt; // ref count
  uint timestamp; // time stamp
  // struct buf *prev; // LRU cache list
  struct buf *next;
  uchar data[BSIZE];
};

