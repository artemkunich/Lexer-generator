#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "datastructs.h"
#include "lexer.h"

#define ID_MAX_LENGTH 50

static char strTemp[100];
static char lexState;
static char lexPart;
static char peek;

char initStr[4096] = "\
d	1|2|3|4|5|6|7|8|9|0\n\
wd	a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z\n\
wu	tA|B|C|D|E|F|G|H|I|J|K|L|M|N|O|P|Q|R|S|T|U|V|W|X|Y|Z\n\
w	{wd}|{wu}|_\n\
s	\\ |\\\t|\\\n\n";
int initStrP;
int initStrFlag;

static void throwExeption(char* msg, ...){
	va_list ptr;	
	va_start(ptr, msg);
	printf("Error on line %i. ", line);
	vprintf(msg, ptr);
	va_end(ptr);
	getchar();
	exit(-1);
}

char* tokenToString(Token* t){
	
	if(t->tag < 256){
		sprintf(strTemp, "%c", (char)t->tag);
		return strTemp;
	}
	
	//if(t->tag == NUM){
	//	sprintf(strTemp, "%n", t->numValue);
	//	return strTemp;		
	//}
	
	return t->lexeme;	
}

char* tokenTagToString(int tag){

	if(tag < 256){
		sprintf(strTemp, "%c", (char)tag);
		return strTemp;
	}
	
	switch(tag){
		case KLEENE_CLOSURE: 	return "*";
		case POSITIVE_CLOSURE: return "+";
		case UNION: return "|";
		case LBRACKET:	return "(";
		case RBRACKET:	return ")";
		case DELIMETR:	return "%%";	
		case ACTION:	return "action";	
		case ID:	return "identifier";
		case ID_DECL:	return "identifier declaration";
		case PROGNOSTIC:	return "/";
		case EOF:	return "EOF";			
	}	
	
	sprintf(strTemp, "%c", (char)tag);
	return strTemp;
}


Token* token(int t){
	Token* token = malloc(sizeof(Token));
	token->tag = t;
	return token; 
}

//Token* tokenNum(int v){
//	Token* token = malloc(sizeof(Token));
//	token->tag = NUM;
//	token->numValue = v;
//	return token; 
//}

Token* tokenWord(int t, char* s){
	Token* token = malloc(sizeof(Token));
	token->tag = t;
	token->lexeme = malloc(strlen(s) + 1);
	strcpy(token->lexeme,s);
	return token;
}

void reserve(HashtableElement** hashtable, Token* t){
	hashtableAdd(hashtable, t->lexeme, t);
}

HashtableElement** idHashtableInitialize(int hashBasis){
	line = 1;
	lexState = 0;
	lexPart = 0;
	HashtableElement** hashtabFstElementPtPt = hashtableCreate(hashBasis);

	reserve(hashtabFstElementPtPt, tokenWord(KLEENE_CLOSURE, "*"));
	reserve(hashtabFstElementPtPt, tokenWord(POSITIVE_CLOSURE, "+"));
	reserve(hashtabFstElementPtPt, tokenWord(UNION, "|"));
	reserve(hashtabFstElementPtPt, tokenWord(LBRACKET, "("));
	reserve(hashtabFstElementPtPt, tokenWord(RBRACKET, ")"));
	reserve(hashtabFstElementPtPt, tokenWord(PROGNOSTIC, "/"));
	reserve(hashtabFstElementPtPt, tokenWord(DELIMETR, "%%"));
	
	return hashtabFstElementPtPt;
}

char readch(FILE* fp){
	if(initStr[initStrP] != '\0'){
		return initStr[initStrP++]; 
	}
	if(initStrFlag == 0) initStrFlag = 1;
	return (char)fgetc(fp);	
}

int readAndEqualch(FILE* fp, char e){
	peek = readch(fp);
	if(peek == e)
		return 1;
	ungetc(peek, fp);
	return 0;	
}

Token* getIdDeclaration(FILE* fp, HashtableElement** hashtable){
	Token* id = NULL;
	if((peek >= 'A' && peek <= 'Z') || (peek >= 'a' && peek <= 'z') || (peek == '_')){		
		int i = 0;		
		do {	
			strTemp[i++] = peek;
			if(i == (ID_MAX_LENGTH + 1)){
				strTemp[i] = '\0';	
				throwExeption("Too long id %s. Max length of id is %i", strTemp, ID_MAX_LENGTH);
			}
			peek = readch(fp);		
		} while((peek >= '0' && peek <= '9') || (peek >= 'A' && peek <= 'Z') || (peek >= 'a' && peek <= 'z') || (peek == '_'));
		
		strTemp[i] = '\0';		
		
		id = hashtableGetValue(hashtable, strTemp);
			
		if (id == NULL){
			id = tokenWord(ID_DECL, strTemp);
			hashtableAdd(hashtable, strTemp, id);
		}
		if(initStrFlag){
			ungetc(peek, fp);			
		} else {
			initStrP--;
		}
	} else {
		throwExeption("Identifier format is not correct: %c", peek);
	}
	return id;	
}

Token* getId(FILE* fp, HashtableElement** hashtable){
	int i = 0;	
	peek = readch(fp);	
	if(peek == '}') throwExeption("Empty identifier is occurred");
	
	do {	
		strTemp[i++] = peek;
		if(i == (ID_MAX_LENGTH + 1)){
			strTemp[i] = '\0';	
			throwExeption("Too long id %s. Max length of id is 50", strTemp);
		}
		peek = readch(fp);
		
		if(peek == '{'){
			throwExeption("Not allowed symbol \'{\' in identifier");
		}
		
	} while(peek != '}');

	strTemp[i] = '\0';		
		
	Token* id = hashtableGetValue(hashtable, strTemp);
		
	if(id == NULL){
		id = tokenWord(ID, strTemp);
		throwExeption("Identifier %s is not declared", strTemp);
	}
	id->tag = ID;	
	return id;		
}

Token* getAction(FILE* fp, HashtableElement** hashtable){
	Token* action;
	if(peek == '{'){	
		int i = 0;	
		peek = readch(fp);	
		if(peek == '}'){
			action = hashtableGetValue(hashtable, "");			
			if (action == NULL){
				action = tokenWord(ACTION, "");
				hashtableAdd(hashtable, "", action);
			}			
			return action;							
		} else {			
			if(peek == '{') throwExeption("Not allowed symbol \'{\' in identifier");			
			do {	
				strTemp[i++] = peek;
				peek = readch(fp);				
				if(peek == '{') throwExeption("Not allowed symbol \'{\' in identifier");						
			} while(peek != '}');
			strTemp[i] = '\0';	
			action = hashtableGetValue(hashtable, strTemp);				
			if (action == NULL){
				action = tokenWord(ACTION, strTemp);
				hashtableAdd(hashtable, strTemp, action);
			}						
		}	
	} else {
		throwExeption("Not allowed symbol \'%i\' in identifier", peek);
	}
	return action;	
}

Token* getNextToken(FILE* fp, HashtableElement** hashtable){	
	peek = readch(fp);
	
	for(int i = 0; 1; i++){
		if(peek == ' ' || peek == '\t' || peek == '\n'){
			if(peek == '\n') {line++;}
			peek = readch(fp);
			if(i == 0){
				if(lexState == 0){ 
					lexState = 1; 
				} else {
					lexState = 0;
				}
			}
			continue;
		}
		else {
			break;
		}
	}

	if(peek == EOF) return token(peek);
	
	if(peek == '%' && readAndEqualch(fp, '%')){
		peek = readch(fp);		
		while(1){
			if(peek == ' ' || peek == '\t' || peek == '\n'){
				if(peek == '\n') {line++;}
				peek = readch(fp);
				continue;
			}
			else {
				break;
			}
		}
		lexPart++;
		lexState = 0;
		ungetc(peek, fp);
		return hashtableGetValue(hashtable, "%%");
	}
	
	if(lexState == 0 && lexPart == 0){	
		return getIdDeclaration(fp, hashtable);			
	} else if((lexState == 1 && lexPart == 0)||(lexState == 0 && lexPart == 1)){

		if(peek == '\\'){	
			peek = readch(fp);
			return token(peek);					
		}
	
		if(peek == '/' && lexPart == 0){	
			throwExeption("Prognostic operator can be used only in the second part of lex");					
		}		
	
		if(peek == '(' || peek == ')' || peek == '*' || peek == '+' || peek == '|' || peek == '/'){
			sprintf(strTemp, "%c", peek);
			return hashtableGetValue(hashtable, strTemp);
		}
				
		if(peek == '{'){
			return getId(fp, hashtable);
		}

		if(peek == '}'){
			throwExeption("Not allowed symbol \'}\'");
		}
		Token* t = token(peek);
		return t;
		
	} else if(lexState == 1 && lexPart == 1){
		return getAction(fp, hashtable);
	} else {
		int i = 0;	
		peek = readch(fp);	
		
		do {	
			strTemp[i++] = peek;
			peek = readch(fp);			
		} while(peek != EOF);
			
		strTemp[i] = '\0';					
		Token* action = tokenWord(ACTION, strTemp);		
		return action;		
	}
	return NULL;
}