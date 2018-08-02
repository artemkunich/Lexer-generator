#define UNION 256
#define KLEENE_CLOSURE 257
#define POSITIVE_CLOSURE 258
#define LBRACKET 259
#define RBRACKET 260
#define DELIMETR 261
#define PROGNOSTIC 262
#define ID 263
#define ID_DECL 264
#define ACTION 265


typedef struct Token Token;
typedef struct HashtableElement HashtableElement;

struct Token{
	int tag;
	int numValue; 		//For integer values
	char* lexeme; 		//for ids and reserved words
};

int line;
Token* token(int);
//Token* tokenNum(int);
Token* tokenWord(int, char*);

char* tokenToString(Token*);
char* tokenTagToString(int);

Token* getNextToken(FILE*, HashtableElement**);
void reserve(HashtableElement**, Token*);
HashtableElement** idHashtableInitialize(int);