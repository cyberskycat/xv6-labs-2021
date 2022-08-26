#include "types.h"
#include "spinlock.h"
#include "buf.h"
#include "param.h"

#define HSIZE 13
struct bcache_segment {
  struct spinlock lock;
  struct buf buf[NBUF];
  struct buf head;
};

struct bhash_table{
 struct  bcache_segment bs[HSIZE];
};

unsigned int hash(unsigned int x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x%HSIZE;
}

// struct buf