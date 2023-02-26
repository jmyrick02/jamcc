#include <stdlib.h>
#include <string.h>

#define FNV_OFFSET_BASIS 0xCBF29CE484222325
#define FNV_PRIME 0x100000001B3

typedef struct SymbolTableEntry {
  char* identifierName;
  int val;
  struct SymbolTableEntry* next;
} SymbolTableEntry;
