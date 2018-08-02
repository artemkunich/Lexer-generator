#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "datastructs.h"
#include "lexer.h"
#include "parser.h"

#define HASHTABLE_BASIS 17

HashtableElement** hashtable;
HashtableElement** idhashtable;

char* getTokenName(int);

static void throwExeption(char* msg, ...){
	va_list ptr;	
	va_start(ptr, msg);
	printf("Error on line %i. ", line);
	vprintf(msg, ptr);
	va_end(ptr);
	getchar();
	exit(-1);
}

/***********Syntax tree initialization****************/

NodeAction* nodeAction(Node* node, char* action){
	NodeAction* nodeAction = malloc(sizeof(NodeAction));
	nodeAction->node = node;
	nodeAction->action = action;
	return nodeAction;
}

Node* nodeUnion(Node* chNode1, Node* chNode2){
	Node* node = malloc(sizeof(Node));
	node->tag = UNION;
	node->chNode1 = chNode1;
	node->chNode2 = chNode2;
	return node;
}

Node* nodeConcat(Node* chNode1, Node* chNode2){
	Node* node = malloc(sizeof(Node));
	node->tag = CONCAT;
	node->chNode1 = chNode1;
	node->chNode2 = chNode2;
	return node;
}

Node* nodeConcatPrognostic(Node* chNode1, Node* chNode2){
	Node* node = malloc(sizeof(Node));
	node->tag = PROGNOSTIC;
	node->chNode1 = chNode1;
	node->chNode2 = chNode2;
	return node;
}

Node* nodeKleeneClosure(Node* chNode){
	Node* node = malloc(sizeof(Node));
	node->tag = KLEENE_CLOSURE;
	node->chNode1 = chNode;
	node->chNode2 = NULL;
	return node;
}

Node* nodePositiveClosure(Node* chNode){
	Node* node = malloc(sizeof(Node));
	node->tag = POSITIVE_CLOSURE;
	node->chNode1 = chNode;
	node->chNode2 = NULL;
	return node;
}

Node* nodeSymbol(Token* token){
	Node* node = malloc(sizeof(Node));
	node->tag = SYMBOL;
	node->token = token;
	node->chNode1 = NULL;
	node->chNode2 = NULL;
	return node;
}

/***********Grammar productions****************/
Token* look;
FILE* sourseCodeFile;

void regexpDecls();
Node* regexpUnion();
Node* regexpConcat();
Node* regexpUnionRest();
Node* regexpConcatRest();
Node* regexpElems();
Node* regexpElem();

void move(){look = getNextToken(sourseCodeFile, hashtable); }

void match(int tag){
	if(tag == look->tag){
		move();
		return;
	}
	throwExeption("Expected token %s, actual token is %s", tokenTagToString(tag), look->lexeme);
}

/*Node* program(FILE* fp){
	sourseCodeFile = (FILE*)fp;
	hashtable = idHashtableInitialize(HASHTABLE_BASIS);
	idhashtable = idHashtableInitialize(HASHTABLE_BASIS);
	move();
	regexpDecls();
	match(DELIMETR);	
	Node* s = regexpUnion();
	if(look->tag == PROGNOSTIC){
		move();
		Node* sc = regexpUnion();
		s = nodeConcatPrognostic(s,sc);
	}
	if(look->tag == ACTION){
		s->action = look->lexeme;
		move();
	}	
	return s;
}*/

void parserInitialize(FILE* fp){
	sourseCodeFile = fp;
	hashtable = idHashtableInitialize(HASHTABLE_BASIS);
	idhashtable = hashtableCreate(HASHTABLE_BASIS);
	move();
	regexpDecls();
	match(DELIMETR);
}

NodeAction* getNextRegexpTemplate(){
	if(look->tag == EOF) return NULL;
	Node* s = regexpUnion();
	NodeAction* na = NULL;
	if(look->tag == PROGNOSTIC){
		move();
		Node* sc = regexpUnion();
		s = nodeConcatPrognostic(s,sc);
	}
	if(look->tag == ACTION){
		na = nodeAction(s,look->lexeme);
		move();
	} else {
		throwExeption("Expected token %s, actual token is %s", tokenTagToString(look->tag), look->lexeme);
	}
	return na;
}

void regexpDecls(){
	printf("regexpDecls %s\n", tokenTagToString(look->tag));
	while(look->tag == ID_DECL){		
		Token* idToken = look; move();	
		hashtableAdd(idhashtable, idToken->lexeme, regexpUnion());			
	}
}

Node* regexpUnion(){
	printf("regexpUnion %s\n", tokenTagToString(look->tag));	
	Node* node1 = regexpConcat();
	Node* node2 = regexpUnionRest();
	if(node2 == NULL) return node1;	
	return nodeUnion(node1, node2);
}

Node* regexpConcat(){
	printf("regexpConcat %s\n", tokenTagToString(look->tag));		
	Node* node1 = regexpElems();
	Node* node2 = regexpConcatRest();
	if(node2 == NULL) return node1;
	return nodeConcat(node1, node2);
}

Node* regexpUnionRest(){
	printf("regexpUnionRest %s\n", tokenTagToString(look->tag));	
	if(look->tag == UNION){
		move();
		Node* node1 = regexpConcat();
		Node* node2 = regexpUnionRest();
		if(node2 == NULL) return node1;
		return nodeUnion(node1, node2);
	}
	return NULL;
}

Node* regexpConcatRest(){
	printf("regexpConcatRest %s\n", tokenTagToString(look->tag));	
	if(look->tag != UNION && look->tag != EOF && look->tag != RBRACKET && look->tag != ID_DECL && look->tag != ACTION && look->tag != DELIMETR && look->tag != PROGNOSTIC){
		Node* node1 = regexpElems();
		Node* node2 = regexpConcatRest();
		if(node2 == NULL) return node1;
		return nodeConcat(node1, node2);
	}
	return NULL;		
}

Node* regexpElems(){
	printf("regexpElems %s\n", tokenTagToString(look->tag));	
	Node* node = regexpElem();
	if(look->tag == KLEENE_CLOSURE){
		move();
		return nodeKleeneClosure(node);
	}
	if(look->tag == POSITIVE_CLOSURE){
		move();
		return nodePositiveClosure(node);
	}
	return node;
}

Node* regexpElem(){
	printf("regexpElem %s\n", tokenTagToString(look->tag));
	Node* node;
	if(look->tag == LBRACKET){
		move();
		node = regexpUnion();
		match(RBRACKET);
		return node;
	} else 
	if(look->tag == ID){
		node = hashtableGetValue(idhashtable, look->lexeme);
		if(node == NULL) throwExeption("Identifier %s is not declared", look->lexeme);
		move();
		return node;
	}
	if(look->tag < 0 || look->tag > 255) throwExeption("Incorrect symbol %s", tokenTagToString(look->tag)); 
	node = nodeSymbol(look);
	move();
	return node;
}

/********************** Syntax tree printing *****************************/

#define STR_MAXLENGTH 200

static char strTemp[STR_MAXLENGTH];

char* nodeTagToString(Node* nodePt){
	if(nodePt->tag < 256){
		sprintf(strTemp, "%c", (char)(nodePt->tag));
		return strTemp;
	}	
	switch(nodePt->tag){
		case UNION:				return "UNION";
		case CONCAT:			return "CONCAT";
		case PROGNOSTIC:	return "PROGNOSTIC";
		case KLEENE_CLOSURE:	return "KLEENE_CLOSURE";	
		case POSITIVE_CLOSURE:	return "POSITIVE_CLOSURE";		
		case SYMBOL:	
			sprintf(strTemp, "SYM(%c)", nodePt->token->tag);
			return strTemp;	
	}
	sprintf(strTemp, "UNDEF_%i", nodePt->tag);
	return strTemp;
}

void printSyntaxTree(Node* nodePt, int indent){
	for(int i = 0; i < indent; i++){
		printf("\t");
	}
	printf("%s\n",nodeTagToString(nodePt));	
	if(nodePt->chNode1 != NULL)printSyntaxTree(nodePt->chNode1, indent + 2);
	if(nodePt->chNode2 != NULL)printSyntaxTree(nodePt->chNode2, indent + 2);
}
