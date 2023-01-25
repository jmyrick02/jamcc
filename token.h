typedef enum {
	PLUS = 0, 
	MINUS, 
	STAR, 
	SLASH, 
	INTEGER_LITERAL,
	END
} TokenType;

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
	int value;
} Token;
