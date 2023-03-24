#include "../../include/generation/symboltable.h"
#include "stdio.h"

SymbolTableEntry** data;
int length = 0;

int hash(char* string) {
  unsigned long int hash = FNV_OFFSET_BASIS;

  int n = strlen(string);
  for (int i = 0; i < n; i++) {
    hash ^= string[i];
    hash *= FNV_PRIME;
  }

  return hash % length;
}

// Recommended: 512
void initSymbolTable(int len) {
  data = calloc(len, sizeof(SymbolTableEntry*)); // calloc initializes to NULL ptrs
  length = len;
}

void updateSymbolTable(SymbolTableEntry entry) {
  SymbolTableEntry* newEntry = malloc(sizeof(SymbolTableEntry));
  strcpy(newEntry->identifierName, entry.identifierName);
  newEntry->type = entry.type;
  newEntry->next = NULL;

  char* identifier = entry.identifierName;

  int hashResult = hash(identifier);

  if (data[hashResult] == NULL) {
    data[hashResult] = newEntry;
  } else {
    SymbolTableEntry* cur = data[hashResult];

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

SymbolTableEntry* getSymbolTableEntry(char* identifier) {
  int hashResult = hash(identifier);

  if (data[hashResult] != NULL) {
    SymbolTableEntry* cur = data[hashResult];

    while (cur != NULL) {
      if (strcmp(cur->identifierName, identifier) == 0)
        return cur;

      cur = cur->next;
    } 
  }

  return NULL;
} 
