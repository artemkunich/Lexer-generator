#define SYMBOL 276
#define CONCAT 277

typedef struct Node Node;
typedef struct NodeAction NodeAction;
typedef struct HashtableElement HashtableElement;

struct NodeAction{
	Node* node;
	char* action;
};

struct Node{
	int tag;
	Token* token;
	Node* chNode1;
	Node* chNode2;
};

NodeAction* getNextRegexpTemplate();
void parserInitialize(FILE*);
void printSyntaxTree(Node*, int);
char* nodeTagToString(Node*);