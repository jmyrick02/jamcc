typedef enum {
	UNKNOWN_TOKEN = 0, 
	PLUS, 
	MINUS, 
	STAR, 
	SLASH, 
	INTEGER_LITERAL
} TokenType;

static const char* TOKENTYPE_STRING[] = {
	"unknown token", // UNKNOWN_TOKEN
	"+", // PLUS
	"-", // MINUS
	"*", // STAR
	"/", // SLASH
	"integer literal" // INTEGER_LITERAL
};

typedef struct Token {
	TokenType type;
	int value;
} Token;
