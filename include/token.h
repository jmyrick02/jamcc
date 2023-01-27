typedef enum {
	PLUS = 0, 
	MINUS, 
	STAR, 
	SLASH, 
	INTEGER_LITERAL,
	END
} TokenType;

static const int PRECEDENCE[] = {
	0, // PLUS
	0, // MINUS
	1, // STAR
	1, // SLASH
	-1, //INTEGER_LITERAL
	-1, // INTEGER_LITERAL
};

static const char* TOKENTYPE_STRING[] = {
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

