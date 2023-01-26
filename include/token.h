typedef enum {
	UNKNOWN = 0,
	PLUS, 
	MINUS, 
	STAR, 
	SLASH, 
	INTEGER_LITERAL,
	END
} TokenType;

static const char* TOKENTYPE_STRING[] = {
	"unknown token", // UNKNOWN
	"+", // PLUS
	"-", // MINUS
	"*", // STAR
	"/", // SLASH
	"integer literal", // INTEGER_LITERAL
	"EOF", // END
};

typedef struct Token {
	TokenType type;
	int val;
} Token;
