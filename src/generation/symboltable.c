#include "../../include/generation/symboltable.h"

SymbolTableEntry** data;
int length = 0;

int hash(char* string) {
  long hash = FNV_OFFSET_BASIS;

  int n = strlen(string);
  for (int i = 0; i < n; i++) {
    hash ^= string[i];
    hash *= FNV_PRIME;
  }

  return hash % length;
}

// Recommended: 512
void initSymbolTable(int len) {
  data = calloc(length, sizeof(SymbolTableEntry*)); // calloc initializes to NULL ptrs
  length = len;
}

// Recommended: 2
void resizeSymbolTable(int factor) {
  SymbolTableEntry **newData = malloc(length * factor * sizeof(SymbolTableEntry*));

  for (int i = 0; i < length; i++) {
    newData[i] = data[i];
  }
  free(data);

  data = newData;
  length *= factor;
}

void updateSymbolTable(char* identifier, int val) {
  int hashResult = hash(identifier);

  if (data[hashResult] == NULL) {
    data[hashResult] = malloc(sizeof(SymbolTableEntry));
    data[hashResult]->identifierName = identifier;
    data[hashResult]->val = val;
    data[hashResult]->next = NULL;
  } else {
    SymbolTableEntry* cur = data[hashResult];

    while (cur->next != NULL) {
      if (strcmp(cur->identifierName, identifier) == 0)
        return;

      cur = cur->next;
    } // We're now at the tail

    SymbolTableEntry* newEntry = malloc(sizeof(SymbolTableEntry));
    newEntry->identifierName = identifier;
    newEntry->val = val;
    newEntry->next = NULL;

    cur->next = newEntry;
  }
}

SymbolTableEntry* getSymbolTableEntry(char* identifier) {
    int hashResult = hash(identifier);

    if (data[hashResult] != NULL) {
      SymbolTableEntry* cur = data[hashResult];

      while (cur->next != NULL) {
        if (strcmp(cur->identifierName, identifier) == 0)
          return cur;

        cur = cur->next;
      } // We're now at the tail
    }

    return NULL;
  }
