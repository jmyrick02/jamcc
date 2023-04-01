#include "../../include/generation/symboltable.h"
#include "stdio.h"

SymbolTableNode *tables;

int hash(int length, char* string) {
  unsigned long int hash = FNV_OFFSET_BASIS;

  int n = strlen(string);
  for (int i = 0; i < n; i++) {
    hash ^= string[i];
    hash *= FNV_PRIME;
  }

  return hash % length;
}

void updateSymbolTable(SymbolTable *table, SymbolTableEntry entry) {
  SymbolTableEntry* newEntry = malloc(sizeof(SymbolTableEntry));
  *newEntry = CONSTRUCTOR_SYMBOL_TABLE_ENTRY(entry.type, NULL, entry.latestLLVMValue);
  strcpy(newEntry->identifierName, entry.identifierName);

  char* identifier = entry.identifierName;

  int hashResult = hash(table->length, identifier);

  if (table->data[hashResult] == NULL) {
    table->data[hashResult] = newEntry;
  } else {
    SymbolTableEntry* cur = table->data[hashResult];

    while (cur != NULL) {
      if (strcmp(cur->identifierName, identifier) == 0) {
        cur->type = newEntry->type;
        return;
      }

      if (cur->next != NULL)
        cur = cur->next;
    } // We're now at the tail

    cur->next = newEntry;
  }
}

SymbolTableEntry* getSymbolTableEntry(SymbolTable *table, char* identifier) {
  int hashResult = hash(table->length, identifier);

  if (table->data[hashResult] != NULL) {
    SymbolTableEntry* cur = table->data[hashResult];

    while (cur != NULL) {
      if (strcmp(cur->identifierName, identifier) == 0)
        return cur;

      cur = cur->next;
    } 
  }

  return NULL;
} 

void initTables() {
  SymbolTable *global = malloc(sizeof(SymbolTable));
  *global = CONSTRUCTOR_SYMBOL_TABLE;

  tables = malloc(sizeof(SymbolTableNode));
  *tables = CONSTRUCTOR_SYMBOL_TABLE_NODE(global, NULL);
}

void pushTable(SymbolTable *table) {
  SymbolTableNode *new = malloc(sizeof(SymbolTableNode));

  if (tables == NULL) {
    *new = CONSTRUCTOR_SYMBOL_TABLE_NODE(table, NULL);
  } else {
    *new = CONSTRUCTOR_SYMBOL_TABLE_NODE(table, tables);
  }

  tables = new;
}

void popTable() {
  tables = tables->next;
}

SymbolTableEntry *getTables(char *identifier) {
  SymbolTableNode *cur = tables;
  while (cur != NULL) {
    SymbolTableEntry *entry = getSymbolTableEntry(cur->table, identifier);
    if (entry != NULL) {
      return entry;
    }

    cur = cur->next;
  }

  return NULL;
}

// Adds entry to local context (top of stack)
void addToTables(SymbolTableEntry entry) {
  updateSymbolTable(tables->table, entry);
}

SymbolTableEntry *getGlobal(char* identifier) {
  SymbolTableNode *cur = tables;
  while (cur->next != NULL) {
    cur = cur->next;
  }

  return getSymbolTableEntry(cur->table, identifier);
}

void addGlobal(SymbolTableEntry entry) {
  SymbolTableNode *cur = tables;
  while (cur->next != NULL) {
    cur = cur->next;
  }

  updateSymbolTable(cur->table, entry);
}

void updateTables(SymbolTableEntry entry) {
  SymbolTableNode *cur = tables;
  while (cur != NULL) {
    if (getSymbolTableEntry(cur->table, entry.identifierName) != NULL) {
      updateSymbolTable(cur->table, entry);
      return;
    }
    cur = cur->next;
  }

  // Identifier does not exist, so we'll insert in the local symbol table (top of stack)
  updateSymbolTable(tables->table, entry);
}
