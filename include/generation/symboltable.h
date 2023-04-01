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
  LLVMValue latestLLVMValue;
} SymbolTableEntry;

#pragma once
typedef struct SymbolTable {
  SymbolTableEntry** data;
  int length;
} SymbolTable;

#pragma once
typedef struct SymbolTableNode {
  SymbolTable* table;
  struct SymbolTableNode *next;
} SymbolTableNode;

void initTables();

void pushTable(SymbolTable *table);

void popTable();

SymbolTableEntry *getTables(char* identifier);

void addToTables(SymbolTableEntry entry);

SymbolTableEntry *getGlobal(char* identifier);

void addGlobal(SymbolTableEntry entry);

void updateTables(SymbolTableEntry entry);

// CONSTRUCTORS:

#define CONSTRUCTOR_SYMBOL_TABLE (SymbolTable) {calloc(512, sizeof(SymbolTableEntry*)), 512}

#define CONSTRUCTOR_SYMBOL_TABLE_NODE(table_ptr, next) (SymbolTableNode) {table_ptr, next}

#define CONSTRUCTOR_SYMBOL_TABLE_ENTRY(type, next, latest_llvmvalue) (SymbolTableEntry) {"", type, next, latest_llvmvalue}
