#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "lexer.h"
#include "parser.h"
#include "convertorRegexp2NFA.h"
#include "generatorOfSimulatorNFA.h"

void throwExeption(char* msg, ...){
	va_list ptr;	
	va_start(ptr, msg);
	
	printf("%s. ","Error");
	vprintf(msg, ptr);
	
	va_end(ptr);
	
	getchar();
	exit(-1);
}

int main(int argc, char** argv){
	if(argc < 2){
		throwExeption("Incorrect number of arguments. Expected %i, actual %i", 2, argc);	
	}
	NodeAction* nodeActionFst;
	TrStNFA** trStNFA;
	char* fileNamePt = argv[1];
	FILE* fpIn = fopen( fileNamePt, "r" );

	parserInitialize(fpIn);
	convertorRegexp2NFAInitialize();
	
	nodeActionFst = getNextRegexpTemplate(fpIn);
	
	while(nodeActionFst != NULL){
		printSyntaxTree(nodeActionFst->node, 0);
		trStNFA = convertRegexp2NFA(nodeActionFst);
		nodeActionFst = getNextRegexpTemplate(fpIn);
	}
	
	fclose(fpIn);		

	//fileNamePt = argv[2];
	//FILE* fpOut = fopen( fileNamePt, "w" );
	//generateOutput(fpOut,nd,0,0); 	//Generate output, 3 address language
	//fclose(fpOut);
	
	//if((argc > 2) && (strcmp(argv[1],"pst"))){
	//	printSyntaxTree(nd, 0); 	//If 3rd parameter == pst, print syntax tree
	//}
	
	printNFA(trStNFA);
	
	for(int i = 0; i <= stateCounter; i++){
		if(finalState[i] != -1){
			printf("state: %i action: %i {%s} state: %i\n",i, finalState[i],action[finalState[i]], prognosticFinalState[finalState[i]]);
		}
	}
	printf("\n");

	if(argc > 2){
		fileNamePt = argv[2];
		FILE* fpOut = fopen( fileNamePt, "w" );
		generateSimulatorNFA(fpOut); 	//Generate output, 3 address language
		fclose(fpOut);
	}
	
	return 0;
}
