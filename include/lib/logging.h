/**
* @file logging.h
* @author Charles Averill
* @brief Function headers, ANSI defines, and enums for logging and raising warnings and errors
* @date 08-Sep-2022
*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define ANSI_BOLD "\033[1m"
#define ANSI_RED "\033[38:5:196m"
#define ANSI_ORANGE "\033[38:5:208m"
#define ANSI_YELLOW "\033[38:5:178m"
#define ANSI_RESET "\033[0m"

#define ERROR_RED ANSI_RED ANSI_BOLD
#define ERROR_ORANGE ANSI_ORANGE ANSI_BOLD
#define ERROR_YELLOW ANSI_YELLOW ANSI_BOLD

/**
* @brief Return codes used in different scenarios
*/
typedef enum {
	RC_OK,
	RC_ERROR,
} ReturnCode;

/**
* @brief String representation of return codes
*/
const static char* returnCodeStrings[] = {
	"OK", "ERROR", 
};

void fatal(ReturnCode rc, const char* fmt, ...);

