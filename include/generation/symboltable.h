#include <stdlib.h>
#include <string.h>

#include "../token.h"
#include "../llvm.h"

#define FNV_OFFSET_BASIS 0xCBF29CE484222325
#define FNV_PRIME 0x100000001B3

#pragma once
typedef struct SymbolTableEntry {
  char identifierName[MAX_IDENTIFIER_LENGTH + 1];
  Type type;
  struct SymbolTableEntry* next;
} SymbolTableEntry;

void initSymbolTable(int len);

void updateSymbolTable(SymbolTableEntry entry);

SymbolTableEntry* getSymbolTableEntry(char* identifier);
