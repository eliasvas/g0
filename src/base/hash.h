#ifndef HASH_H__
#define HASH_H__
#include "helper.h"

static u64 djb2(u8 *s) {
  u64 hash = 5381;
  int c;
  while ((c = *s++)) {
    if (is_upper(c)) {
      c = c + 32;
    }
    hash = ((hash << 5) + hash) + c;
  }
  return hash;
}

#endif
