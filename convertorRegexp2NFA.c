#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "datastructs.h"
#include "lexer.h"
#include "parser.h"
#include "convertorRegexp2NFA.h"

#define HASHTABLE_BASIS 17

#define DEF_ACT_MAX_SIZE 15
#define DEF_SYM_MAX_SIZE 15
#define DEF_STA_MAX_SIZE 50

/************************* Regexp to NFA convertor *****************************/

static int actionMaxSize;
static int stateMaxSize;
static int currentFinalState;
static int prognosticFlag;

TrStNFA* newTrStNFA(int state){
	TrStNFA* newState = malloc(sizeof(TrStNFA));
	newState->state = state;
	newState->altState = NULL;
	return newState;
}

void addTrStNFA(int state, int symbol, int newState){
	if(transitionTableNFA[state * symbolMaxSize + symbol] == NULL){
		//printf("addTrStNFA1 %i %i %c\n", state, newState, indexSymbolArray[symbol]);
		transitionTableNFA[state * symbolMaxSize + symbol] = newTrStNFA(newState);
	} else {
		//printf("addTrStNFA2 %i %i %c\n", state, newState, indexSymbolArray[symbol]);
		TrStNFA* trSt = transitionTableNFA[state * symbolMaxSize + symbol];
		while(trSt->altState != NULL){
			if(trSt->state == newState) return;
			//printf("addTrStNFA3 %%i %i %c\n", state, newState, indexSymbolArray[symbol]);
			trSt = trSt->altState;
		}	
		trSt->altState = newTrStNFA(newState);
	}
}

void reallocTransitionTableNFA(int oldStateMaxSize, int oldSymbolMaxSize){	
	//printf("reallocTrStTable\n");
	TrStNFA** newTransitionTableNFA = malloc(symbolMaxSize * stateMaxSize * sizeof(TrStNFA*));
	
	for(int i = 0; i < stateMaxSize; i++){
		for(int j = 0; j < symbolMaxSize; j++){
			if(i < oldStateMaxSize && j < oldSymbolMaxSize){
				newTransitionTableNFA[i * symbolMaxSize + j] = transitionTableNFA[i * oldSymbolMaxSize + j];
			} else {
				newTransitionTableNFA[i * symbolMaxSize + j] = NULL;
			}
		}
	}
	
	free(transitionTableNFA);
	transitionTableNFA = newTransitionTableNFA;
}

int increaseSymbolCounter(){
	symbolCounter++;
	if(symbolCounter >= symbolMaxSize){
		int oldSymbolMaxSize = symbolMaxSize;
		symbolMaxSize *= 2;
		reallocTransitionTableNFA(stateMaxSize, oldSymbolMaxSize);
	}	
	return symbolCounter;
}

int increaseStateCounter(){
	stateCounter++;
	if(stateCounter >= stateMaxSize){
		int oldStateMaxSize = stateMaxSize;
		stateMaxSize *= 2;
		reallocTransitionTableNFA(oldStateMaxSize, symbolMaxSize);
		
		int* newFinalState = malloc(stateMaxSize * sizeof(int));
		for(int i = 0; i < oldStateMaxSize; i++) newFinalState[i] = finalState[i];
		for(int i = oldStateMaxSize; i < stateMaxSize; i++) newFinalState[i] = -1;
		free(finalState);
		finalState = newFinalState;
		
		int** newPrognosticState = malloc(stateMaxSize * sizeof(int));
		for(int i = 0; i < oldStateMaxSize; i++) newPrognosticState[i] = prognosticState[i];
		for(int i = oldStateMaxSize; i < stateMaxSize; i++) newPrognosticState[i] = NULL;
		free(prognosticState);
		prognosticState = newPrognosticState;
	}
	return stateCounter;
}

int increaseActionCounter(){
	actionCounter++;
	if(actionCounter >= actionMaxSize){
		int oldActionMaxSize = actionMaxSize;
		actionMaxSize *= 2;
		
		char** newAction = malloc(sizeof(char*)*actionMaxSize);
		for(int i = 0; i < oldActionMaxSize; i++) newAction[i] = action[i];
		for(int i = oldActionMaxSize; i < actionMaxSize; i++) newAction[i] = NULL;
		free(action);
		action = newAction;

		int* newPrognosticFinalState = malloc(sizeof(int)*actionMaxSize);		
		for(int i = 0; i < oldActionMaxSize; i++) newPrognosticFinalState[i] = prognosticFinalState[i];
		for(int i = oldActionMaxSize; i < actionMaxSize; i++) newPrognosticFinalState[i] = -1;
		free(prognosticFinalState);
		prognosticFinalState = newPrognosticFinalState;
	}
	return stateCounter;
}

void regexp2NFA(Node*);

void prognostic2NFA(Node* node){
	//printf("positiveClosure2NFA\n");
	regexp2NFA(node->chNode1);
	int fstChNode2State = stateCounter;
	addTrStNFA(fstChNode2State, 1, increaseStateCounter());
	prognosticFlag = 1;
	regexp2NFA(node->chNode2);
	prognosticFlag = 0;
}

void positiveClosure2NFA(Node* node){
	//printf("positiveClosure2NFA\n");
	int startState = stateCounter;
	
	regexp2NFA(node->chNode1);
	
	int lstChNodeState = stateCounter;
	addTrStNFA(lstChNodeState, 0, startState);
	
	currentFinalState = lstChNodeState;
}

void kleeneClosure2NFA(Node* node){
	//printf("kleeneClosure2NFA\n");
	int startState = stateCounter;
	int scdState = increaseStateCounter();	
	addTrStNFA(startState, 0, scdState);
	
	regexp2NFA(node->chNode1);
	
	int lstChNodeState = stateCounter;
	addTrStNFA(lstChNodeState, 0, scdState);
	
	increaseStateCounter();	
	
	addTrStNFA(lstChNodeState, 0, stateCounter);	
	addTrStNFA(startState, 0, stateCounter);
	currentFinalState = stateCounter;
}

void concat2NFA(Node* node){
	//printf("concat2NFA\n");
	regexp2NFA(node->chNode1);
	regexp2NFA(node->chNode2);
	currentFinalState = stateCounter;
}

void union2NFA(Node* node){
	//printf("union2NFA\n");
	int startState = stateCounter;

	int fstChNode1State = increaseStateCounter();		
	regexp2NFA(node->chNode1);
	int lstChNode1State = stateCounter;	
	
	int fstChNode2State = increaseStateCounter();
	regexp2NFA(node->chNode2);
	int lstChNode2State = stateCounter;
	
	addTrStNFA(startState, 0, fstChNode1State);
	addTrStNFA(startState, 0, fstChNode2State);
	
	increaseStateCounter();
	
	addTrStNFA(lstChNode1State, 0, stateCounter);
	addTrStNFA(lstChNode2State, 0, stateCounter);
	currentFinalState = stateCounter;
}

void symbol2NFA(Node* node){	
	//printf("symbol2NFA\n");
	int symbol = node->token->tag;
	
	if(symbolIndexArray[symbol] == -1){
		symbolIndexArray[symbol] = increaseSymbolCounter();
		indexSymbolArray[symbolCounter] = symbol;
	}
	
	int startState = stateCounter;
	increaseStateCounter();

	addTrStNFA(startState, symbolIndexArray[symbol], stateCounter);	
	if(prognosticFlag) prognosticState[stateCounter] = &prognosticFinalState[actionCounter];
	currentFinalState = stateCounter;	
}

void regexp2NFA(Node* node){
	//printf("regexp2NFA\n");
	switch(node->tag){
		case SYMBOL: symbol2NFA(node); break;
		case CONCAT: concat2NFA(node); break;
		case UNION: union2NFA(node); break;
		case KLEENE_CLOSURE: kleeneClosure2NFA(node); break;
		case POSITIVE_CLOSURE: positiveClosure2NFA(node); break;
		case PROGNOSTIC: prognostic2NFA(node); break;
	}
}

void convertorRegexp2NFAInitialize(){
	symbolMaxSize = DEF_SYM_MAX_SIZE;
	stateMaxSize = DEF_STA_MAX_SIZE;
	actionMaxSize = DEF_ACT_MAX_SIZE;
	
	for(int i = 0; i < 256; i++) symbolIndexArray[i] = -1;
	for(int i = 0; i < 258; i++) indexSymbolArray[i] = -1;
	
	indexSymbolArray[0] = E_TRANS;
	increaseSymbolCounter();
	indexSymbolArray[1] = E_TRANS;
	
	transitionTableNFA = malloc(symbolMaxSize * stateMaxSize * sizeof(TrStNFA*));
	finalState = malloc(stateMaxSize * sizeof(int));	
	prognosticState = malloc(stateMaxSize * sizeof(int));	
	
	for(int i = 0; i < stateMaxSize * symbolMaxSize; i++) transitionTableNFA[i] = NULL;
	for(int i = 0; i < stateMaxSize; i++) finalState[i] = -1;
	for(int i = 0; i < stateMaxSize; i++) prognosticState[i] = NULL;
	
	action = malloc(sizeof(char*) * DEF_ACT_MAX_SIZE);
	prognosticFinalState = malloc(sizeof(int) * DEF_ACT_MAX_SIZE);
}

TrStNFA** convertRegexp2NFA(NodeAction* nodeAction){
	addTrStNFA(0, 0, ++stateCounter);
	regexp2NFA(nodeAction->node);
	finalState[currentFinalState] = actionCounter;
	action[actionCounter] = nodeAction->action;
	prognosticFinalState[actionCounter] = currentFinalState;
	increaseActionCounter();	
	return transitionTableNFA;
}

/********************** NFA printing *****************************/

void printNFA(TrStNFA** trStNFA){
	TrStNFA* tmpTrStNFA;
	
	printf(" ");		
	for(int i = 0; i <= symbolCounter; i++){
		printf("%c ", indexSymbolArray[i]);		
	}
	printf("\n");
	
	for(int i = 0; i <= stateCounter; i++){
		printf("%i ", i);
		for(int j = 0; j <= symbolCounter; j++){		
			if( trStNFA[i * symbolMaxSize + j] == NULL ){
				printf("N ");
				continue;
			}
			tmpTrStNFA = trStNFA[i * symbolMaxSize + j];
			printf("{");
			while(tmpTrStNFA->altState != NULL){
				printf("%i ", tmpTrStNFA->state);
				tmpTrStNFA = tmpTrStNFA->altState;
			}
			printf("%i", tmpTrStNFA->state);
			printf("} ");
		}
		printf("\n");
	}
}