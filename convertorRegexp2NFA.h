#define E_TRANS 256
#define EP_TRANS 257

typedef struct TransitionStateNFA TrStNFA;

int stateCounter;
int symbolCounter;
int symbolMaxSize;
int symbolIndexArray[256];
int indexSymbolArray[257];
int* finalState;
int** prognosticState;
char** action;
int* prognosticFinalState;
int actionCounter;
TrStNFA** transitionTableNFA;

struct TransitionStateNFA{
	int state;
	TrStNFA* altState;
};

void convertorRegexp2NFAInitialize();
TrStNFA** convertRegexp2NFA(NodeAction*);
void printNFA(TrStNFA**);