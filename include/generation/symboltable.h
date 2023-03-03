#include <stdlib.h>
#include <string.h>

#include "../token.h"
#include "../llvm.h"

#define FNV_OFFSET_BASIS 0xCBF29CE484222325
#define FNV_PRIME 0x100000001B3

#pragma once
typedef struct SymbolTableEntry {
  char identifierName[MAX_IDENTIFIER_LENGTH + 1];
  int val;
  NumberType numType;
  struct SymbolTableEntry* next;
} SymbolTableEntry;

void initSymbolTable(int len);

void updateSymbolTable(char* identifier, int val, NumberType numType);

SymbolTableEntry* getSymbolTableEntry(char* identifier);
