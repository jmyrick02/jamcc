/**
 * @file logging.c
 * @author Charles Averill
 * @brief Logging, warnings, and fatal error handling
 * @date 08-Sep-2022
 */

#include "../../include/lib/logging.h"
#include "../../include/token.h"

/**
 * @brief Raises a fatal error that will exit the program
 * 
 * @param rc Return code to exit with
 * @param fmt Format string for printed error
 * @param ... Varargs for printed error
 */

extern int CUR_LINENUM;
extern char CUR_FUNCTION_NAME[MAX_IDENTIFIER_LENGTH + 1];

void fatal(ReturnCode rc, const char* fmt, ...)
{
    va_list func_args;

    va_start(func_args, fmt);
    fprintf(stderr, "%s%s (function %s) (Line number %d)%s", ERROR_RED "[", returnCodeStrings[rc], CUR_FUNCTION_NAME, CUR_LINENUM, "] - " ANSI_RESET);
    vfprintf(stderr, fmt, func_args);
    va_end(func_args);

    // Print fence for error distinguishing
    fprintf(stderr, "\n----------------------------------------\n");

    exit(rc);
}

