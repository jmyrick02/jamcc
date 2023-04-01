#include <stdio.h>

#include "token.h"
#include "generation/symboltable.h"

#define MAX_CHAR_VALUE 127
#define MAX_SHORT_VALUE 32767
#define MAX_INT_VALUE 2147483647
#define MAX_LONG_VALUE 9223372036854775807

void scan();
