#pragma once

struct memcpy {
  char *dst;
  char *src;
  char len;
};

struct memset {
  char *dst;
  char len;
  char val;
};

void MEM_memcpy(struct memcpy *memcpy);
void MEM_memset(struct memset *memset);