typedef struct HashtableElement HashtableElement;
struct HashtableElement {
	char* key;
	void* value;
	struct HashtableElement* next;
};

HashtableElement** hashtableCreate(int); 
void hashtableAdd(struct HashtableElement**, char*, void*);
HashtableElement* hashtableGet(struct HashtableElement**, char*);
void* hashtableGetValue(struct HashtableElement**, char*);
int hashtableDeleteElement(struct HashtableElement**, char*);
void hashtableDelete(struct HashtableElement**);

typedef struct Stack Stack;
struct Stack {
	int size;
	int stackPointer;
	void** stackFrsElement;
};

Stack* stackCreate(int);
void stackDelete(struct Stack*);
void stackPush(struct Stack*, void*); 
void* stackPop(struct Stack*); 