typedef enum {
	UNKNOWN = 0,
	PLUS, 
	MINUS, 
	STAR, 
	SLASH, 
	INTEGER_LITERAL,
	END
} TokenType;

static const int PRECEDENCE[] = {
	-1, // UNKNOWN
	0, // PLUS
	0, // MINUS
	1, // STAR
	1, // SLASH
	-1, //INTEGER_LITERAL
	-1, // INTEGER_LITERAL
};

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

