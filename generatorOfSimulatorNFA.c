#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "lexer.h"
#include "parser.h"
#include "convertorRegexp2NFA.h"
#include "generatorOfSimulatorNFA.h"

void generateSimulatorNFA(FILE* fp){
	
tmpStr = "#include <stdio.h>\n\
#include <stdlib.h>\n\
#include <string.h>\n\
#include <stdarg.h>\n\
\n\
FILE* inputFP;\n\
\n\
char buffer1[4096];\n\
char buffer2[4096];\n\
char* currentBuffer;\n\
int lexemeBegin;\n\
int forward;\n\
int oldStates[";
	fprintf(fp,tmpStr);
	fprintf(fp,"%i",stateCounter+1);
	
tmpStr = "];\n\
int newStates[";
	fprintf(fp,tmpStr);	
	fprintf(fp,"%i",stateCounter+1);	
	
tmpStr = "];\n\n\
int alreadyOn[";
	fprintf(fp,tmpStr);	
	fprintf(fp,"%i",stateCounter+1);	

fprintf(fp,"] = {0");
for(int i = 1; i <= stateCounter; i++) fprintf(fp,",0");
fprintf(fp,"};\n");
	
tmpStr = "int finalStates[";
	fprintf(fp,tmpStr);	
	fprintf(fp,"%i",stateCounter+1);

fprintf(fp,"] = {%i", finalState[0]);
for(int i = 1; i <= stateCounter; i++) fprintf(fp,",%i",finalState[i]);
fprintf(fp,"};\n");


tmpStr = "int prognosticStates[";
	fprintf(fp,tmpStr);	
	fprintf(fp,"%i",stateCounter+1);	

if(prognosticState[0] == NULL){
	fprintf(fp,"] = {%i", -1);
} else {
	fprintf(fp,",%i",*prognosticState[0]);
}
for(int i = 1; i <= stateCounter; i++){
	if(prognosticState[i] == NULL){
		fprintf(fp,",%i",-1);
	} else {
		fprintf(fp,",%i",*prognosticState[i]);
	}
}
fprintf(fp,"};\n\n");

//getPrognosticLength
for(int i = 0; i <= stateCounter; i++) if(finalState[i] > -1) fprintf(fp,"int prognosticLength%i;\n", i);
tmpStr = "\nint* getPrognosticLength(int state){\n\
\tswitch(state){\n";
fprintf(fp,tmpStr);
for(int i = 0; i <= stateCounter; i++) if(finalState[i] > -1) fprintf(fp,"\t\tcase %i: return &prognosticLength%i;\n", i, i);
fprintf(fp,"\t}\n\treturn NULL;\n}\n\n");	

//getPrognosticLengthTmp
for(int i = 0; i <= stateCounter; i++) if(finalState[i] > -1) fprintf(fp,"int getPrognosticLengthTmp%i;\n", i);
tmpStr = "\nint* getPrognosticLengthTmp(int state){\n\
\tswitch(state){\n";
fprintf(fp,tmpStr);
for(int i = 0; i <= stateCounter; i++) if(finalState[i] > -1) fprintf(fp,"\t\tcase %i: return &getPrognosticLengthTmp%i;\n", i, i);
fprintf(fp,"\t}\n\treturn NULL;\n}\n\n");	

//getPrognosticLengthSaved
for(int i = 0; i <= stateCounter; i++) if(finalState[i] > -1) fprintf(fp,"int getPrognosticLengthSaved%i;\n", i);
tmpStr = "\nint* getPrognosticLengthSaved(int state){\n\
\tswitch(state){\n";
fprintf(fp,tmpStr);
for(int i = 0; i <= stateCounter; i++) if(finalState[i] > -1) fprintf(fp,"\t\tcase %i: return &getPrognosticLengthSaved%i;\n", i, i);
fprintf(fp,"\t}\n\treturn NULL;\n}\n\n");	


tmpStr = "int oldStatesCounter;\n\
int newStatesCounter;\n\
\n\
char lexeme[256];\n\n\
int peek;\n\
int lexemeCounter;\n\
\n\
int finalPeekForward;\n\
char* finalPeekBuffer;\n\
\n\
int finalPeekNumber;\n\
int finalPeekStates[";
	fprintf(fp,tmpStr);	
	fprintf(fp,"%i",stateCounter+1);
	
tmpStr = "];\n\
int finalPeekStatesCounter;\n\n\
int symbolIndexArray[256]";
	fprintf(fp,tmpStr);
	
fprintf(fp," = {%i", symbolIndexArray[0]);
for(int i = 1; i < 256; i++) fprintf(fp,",%i",symbolIndexArray[i]);
fprintf(fp,"};\n\n");


TrStNFA* tmpTrStNFA;
int stCounter;

for(int i = 0; i <= stateCounter; i++){
	for(int j = 0; j <= symbolCounter; j++){
		if(transitionTableNFA[i * symbolMaxSize + j] != NULL){
			fprintf(fp,"int nst_%i_%i[] = ",i,j);
			
			tmpTrStNFA = transitionTableNFA[i * symbolMaxSize + j];
			stCounter = 1;
			while(tmpTrStNFA->altState != NULL){
				tmpTrStNFA = tmpTrStNFA->altState;
				stCounter++;
			}			
			
			tmpTrStNFA = transitionTableNFA[i * symbolMaxSize + j];
			fprintf(fp,"{%i,",stCounter);	
			
			while(tmpTrStNFA->altState != NULL){
				fprintf(fp,"%i,", tmpTrStNFA->state);
				tmpTrStNFA = tmpTrStNFA->altState;
			}
			fprintf(fp,"%i", tmpTrStNFA->state);
			fprintf(fp,"};\n");				
		}
	}		
}


fprintf(fp,"\nint line;\n\n");
tmpStr = "static void throwException(char* msg, ...){\n\
\tva_list ptr;\n\
\tva_start(ptr, msg);\n\
\tprintf(\"Error on line %%i. \", line);\n\
\tvprintf(msg, ptr);\n\
\tva_end(ptr);\n\
\tgetchar();\n\
\texit(-1);\n\
}\n\n\
void doAction(int actionNumber){\n\
\tswitch(actionNumber){\n";	
fprintf(fp,tmpStr);

for(int i = 0; i < actionCounter; i++){	
	fprintf(fp,"\t\tcase %i:\n\t\t\t%s\n\t\t\treturn;\n", i, action[i]);	
}

fprintf(fp,"\t\tdefault:\n\t\t\treturn;\n");	

	
tmpStr = "\t}\n\
}\n\
\n\
int* move(int state, int symbolNumber){\n";
fprintf(fp,tmpStr);		

for(int i = 0; i <= stateCounter; i++){
	fprintf(fp,"\tif(state == %i){\n",i);
	for(int j = 0; j <= symbolCounter; j++){
		if(transitionTableNFA[i * symbolMaxSize + j] != NULL){
			fprintf(fp,"\t\tif(symbolNumber == %i){\n",j);
			fprintf(fp,"\t\t\treturn nst_%i_%i;\n",i,j);			
			fprintf(fp,"\t\t}\n");
		}
	}
	fprintf(fp,"\t\treturn NULL;\n");		
	fprintf(fp,"\t} else");			
}
fprintf(fp,"\t{\n\t\treturn NULL;\n\t}\n");
	
		
//addState
	
tmpStr = "}\n\
\n\
void addState(int state){\n\
\tnewStates[++newStatesCounter] = state;\n\
\talreadyOn[state] = 1;\n\
\tint* nextStates = move(state, 0);\n\
\tif(nextStates != NULL){\n\
\t\tfor(int i = 1; i <= nextStates[0]; i++){\n\
\t\t\tif( !alreadyOn[nextStates[i]]) addState(nextStates[i]);\n\
\t\t}\n\
\t}\n\
\tnextStates = move(state, 1);\n\
\tif(nextStates != NULL){\n\
\t\tfor(int i = 1; i <= nextStates[0]; i++){\n\
\t\t\tif( !alreadyOn[nextStates[i]]) addState(nextStates[i]);\n\
\t\t}\n\
\t}\n\
}\n\
\n";
fprintf(fp,tmpStr);

tmpStr = "void fillNewStates(int symbolNumber){\n\
\tint* nextStates;\n\
\tfor(int i = 0; i <= oldStatesCounter; i++){\n\
\t\tnextStates = move(oldStates[i],symbolNumber);\n\
\t\tif(nextStates != NULL){\n\
\t\t\tfor(int i = 1; i <= nextStates[0]; i++){\n\
\t\t\t\tif(!alreadyOn[nextStates[i]]) addState(nextStates[i]);\n\
\t\t\t}\n\
\t\t}\n\
\t}\n\
}\n\
\n";
fprintf(fp,tmpStr);

tmpStr = "void resetNewStates(){\n\
\tint localFinalPeekStatesCounter = -1;\n\
\n\
\tfor(int i = 0; i <= newStatesCounter; i++){\n\
\t\toldStates[i] = newStates[i];\n\
\t\tif(finalStates[newStates[i]] > -1) finalPeekStates[++localFinalPeekStatesCounter] = newStates[i];\n\
\t\tif(prognosticStates[newStates[i]] > -1){\n\
\t\t\t(*(getPrognosticLength(prognosticStates[newStates[i]])))++;\n\
\t\t\t(*(getPrognosticLengthTmp(prognosticStates[newStates[i]])))++;\n\
\t\t};\n\
\t\talreadyOn[newStates[i]] = 0;\n\
\t}\n\
\tif(localFinalPeekStatesCounter != -1){\n\
\t\tfinalPeekStatesCounter = localFinalPeekStatesCounter;\n\
\t\tfinalPeekNumber = lexemeCounter;\n\
\t\tfinalPeekForward = forward;\n\
\t\tfinalPeekBuffer = currentBuffer;\n\
\t}\n\
\toldStatesCounter = newStatesCounter;\n\
\tnewStatesCounter = -1;\n\
\tfor(int i = 0; i < %i; i++){\n\
\t\tif(getPrognosticLength(prognosticStates[i]) != NULL){\n\
\t\t\tif(*(getPrognosticLengthTmp(prognosticStates[i])) == 0){\n\
\t\t\t\t*(getPrognosticLength(prognosticStates[i])) = 0;\n\
\t\t\t}\n\
\t\t}\n\
\t}\n\
\t\n\
\tfor(int i = 0; i < %i; i++){\n\
\t\tif(getPrognosticLengthTmp(prognosticStates[i]) != NULL){\n\
\t\t\t*(getPrognosticLengthTmp(prognosticStates[i])) = 0;\n\
\t\t}\n\
\t}\n\
\t\n\
\tfor(int i = 0; i <= localFinalPeekStatesCounter; i++){\n\
\t\tif(getPrognosticLength(prognosticStates[finalPeekStates[i]]) != NULL){\n\
\t\t\t(*(getPrognosticLengthSaved(prognosticStates[finalPeekStates[i]]))) = (*(getPrognosticLength(prognosticStates[finalPeekStates[i]]))) - (*(getPrognosticLengthSaved(prognosticStates[finalPeekStates[i]])));\n\
\t\t}\n\
\t}\n\
}\n\
\n";
fprintf(fp,tmpStr, stateCounter + 1, stateCounter + 1);

tmpStr = "int increaseForward(){\n\
\tif(currentBuffer[++forward] == EOF){\n\
\t\tif(forward == 4095){\n\
\t\t\tif(currentBuffer == buffer1){\n\
\t\t\t\tcurrentBuffer = buffer2;\n\
\t\t\t\tfread(buffer2,1,4095,inputFP);\n\
\t\t\t} else {\n\
\t\t\t\tcurrentBuffer = buffer1;\n\
\t\t\t\tfread(buffer1,1,4095,inputFP);\n\
\t\t\t}\n\
\t\t\tforward = 0;\n\
\t\t\tcurrentBuffer[strlen(currentBuffer)] = EOF;\n\
\t\t} else {\n\
\t\t\treturn EOF;\n\
\t\t}\n\
\t}\n\
\treturn currentBuffer[forward];\n\
}\n\
\n";
fprintf(fp,tmpStr);

tmpStr = "int getNextPeekNumber(){\n\
\tpeek = increaseForward();\n\
\tif(peek == EOF) return -1;\n\
\treturn symbolIndexArray[peek];\n\
}\n\
\n";
fprintf(fp,tmpStr);

tmpStr = "void getNextToken(){\n\
\tint peekNumber = getNextPeekNumber();\n\
\tfillNewStates(peekNumber);\n\
\n\
\twhile(newStatesCounter != -1){\n\
\t\tlexeme[++lexemeCounter] = (char)peek;\n\
\t\tresetNewStates();\n\
\t\tpeekNumber = getNextPeekNumber();\n\
\t\tfillNewStates(peekNumber);\n\
\t}\n\
\n\
\tif( finalPeekStatesCounter == -1 ){\n\
\t\tthrowException(\"There is not found any template!\");\n\
\t}\n\
\tint actionNumber = 999999;\n\
\tint finalPeekState;\n\
\tfor(int i = 0; i <= finalPeekStatesCounter; i++){\n\
\t\tif( finalStates[finalPeekStates[i]] < actionNumber ) {\n\
\t\t\tactionNumber = finalStates[finalPeekStates[i]];\n\
\t\t\tfinalPeekState = finalPeekStates[i];\n\
\t\t}\n\
\t}\n\
\tif(getPrognosticLength(finalPeekState) != NULL){\n\
\t\tfinalPeekNumber = lexemeCounter - *(getPrognosticLength(finalPeekState)) + *(getPrognosticLengthSaved(finalPeekState));\n\
\t\tfinalPeekForward = lexemeBegin + finalPeekNumber;\n\
\t\tif( finalPeekForward > forward){\n\
\t\t\tfinalPeekForward = finalPeekForward % 4095;\n\
\t\t\tif(currentBuffer == buffer1){\n\
\t\t\t\tcurrentBuffer = buffer2;\n\
\t\t\t} else {\n\
\t\t\t\tcurrentBuffer = buffer1;\n\
\t\t\t}\n\
\t\t}\n\
\t}\n\
\tlexeme[finalPeekNumber + 1] = '\\0';\n\
\tdoAction(actionNumber);\n\
}\n\
\n";
fprintf(fp,tmpStr);

/*
\tif(finalStatesProgState[finalPeekState] > -1){\n\
\t\tfinalPeekNumber = finalStatesProgState[finalPeekState];\n\
\t\tfinalPeekForward = lexemeBegin + finalPeekNumber;\n\
\t}\n\*/

tmpStr = "int initializeSimulatorNFA(FILE* fp){\n\
\tinputFP = fp;\n\
\toldStatesCounter = -1;\n\
\tnewStatesCounter = -1;\n\
\tlexemeCounter = -1;\n\
\tfinalPeekStatesCounter = -1;\n\
\tforward = -1;\n\
\tlexemeBegin = 0;\n\
\tif(fread(buffer1,1,4095,inputFP) == 0) return 0;\n\
\tbuffer1[4095] = EOF;\n\
\tbuffer2[4095] = EOF;\n\
\tcurrentBuffer = buffer1;\n\
\tcurrentBuffer[strlen(currentBuffer)] = EOF;\n\
\tif(currentBuffer[lexemeBegin] == EOF) return 0;\n\
\taddState(0);\n\
\tresetNewStates();\n\
\treturn 1;\n\
}\n";
fprintf(fp,tmpStr);

tmpStr = "int reinitializeSimulatorNFA(){\n\
\toldStatesCounter = -1;\n\
\tnewStatesCounter = -1;\n\
\tforward = finalPeekForward;\n\
\tcurrentBuffer = finalPeekBuffer;\n\
\tlexemeBegin = (forward + 1) % 4096;\n\
\tif(currentBuffer[lexemeBegin] == EOF) return 0;\n\
\tlexemeCounter = -1;\n\
\tfinalPeekStatesCounter = -1;\n\
\taddState(0);\n\
\tresetNewStates();\n\
\treturn 1;\n\
}\n";
fprintf(fp,tmpStr);
}

