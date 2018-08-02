#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "datastructs.h"
#define HASH_X 10


/********************Hashtable********************/

long long mod(long a, long e, long m);
int getHash(char*, int, int);

void hashtableAdd(struct HashtableElement** firstElement, char* key, void* value)
{
	HashtableElement* foundElement = hashtableGet(firstElement, key);

	if (foundElement != NULL)
	{
		foundElement->value = value;
		return;
	}

	int hash = getHash(key, HASH_X, (int)*(firstElement - 1));

	char* copyKey = (char*)malloc(strlen(key) + 1);
	strcpy(copyKey, key);

	//char* copyValue = (char*)malloc(strlen(value) + 1);
	//strcpy(copyValue, value);
	//itoa(hash, copyValue,10);

	HashtableElement* newElement = (HashtableElement*)malloc(sizeof(HashtableElement));
	*newElement = (HashtableElement){ copyKey, value, *(firstElement + hash) };
	*(firstElement + hash) = newElement;
}

HashtableElement* hashtableGet(struct HashtableElement** firstElement, char* key)
{
	int hash = getHash(key, HASH_X, (int)*(firstElement - 1));

	HashtableElement* tmp = *(firstElement+hash);
	if (tmp == NULL)
		return NULL;

	while((strcmp(tmp->key,key) != 0))
	{ 
		if (tmp->next == NULL)
			return NULL;
		tmp = tmp->next;
	}
	return tmp;
}

void* hashtableGetValue(struct HashtableElement** firstElement, char* key)
{
	HashtableElement* tmp = hashtableGet(firstElement, key);
	if (tmp == NULL)
		return NULL;
	return tmp->value;
}

int hashtableDeleteElement(struct HashtableElement** firstElement, char* key)
{
	int hash = getHash(key, HASH_X, (int)*(firstElement - 1));

	HashtableElement* element = *(firstElement + hash);
	HashtableElement* elementPrevious = NULL;

	if (element == NULL)
		return 0;

	while ((strcmp(element->key, key) != 0))
	{
		if (element->next == NULL)
			return 0;

		elementPrevious = element;
		element = element->next;
	}

	if (elementPrevious == NULL)
		*(firstElement + hash) = element->next;
	else
		elementPrevious->next = element->next;
	
	free(element->value);
	free(element->key);
	free(element);
	return 1;
}

void hashtableDelete(struct HashtableElement** firstElement)
{
	int hashTableBasis = (int)*(firstElement - 1);
	HashtableElement* element; 
	
	for(int i = 0; i < hashTableBasis; i++){
		
		while(1){
			element = firstElement[i];
			
			if(element == NULL){
				break;
			}
			
			firstElement[i] = element->next;
			
			free(element->value);
			free(element->key);
			free(element);
		}
	}
	free(firstElement - 1);
}

HashtableElement** hashtableCreate(int arrSize) 
{
	HashtableElement** firstElement = (HashtableElement**)malloc(sizeof(HashtableElement*) * (arrSize + 1));
	*firstElement = (HashtableElement*)arrSize;
	firstElement++;
	
	for (int i = 0; i < arrSize; i++)
	{
		*(firstElement + i) = NULL;
	}
	return firstElement;
}

long long mod(long a, long e, long m)
{
	long long result = 1;
	for (int i = 1; i <= e; i++)
	{
		result *= a;
		result = fmodl(result, m);
	}
	return result;
}

int getHash(char* key, int x, int m)
{
	int hash = 0;
	int i = 0;

	while (key[i] != '\0')
	{
		hash += mod(((int)key[i] * mod(x, i, m)), 1, m);
		hash = mod(hash, 1, m);
		i++;
	}
	return hash;
}


/**********************Stack**********************/

Stack* stackCreate(int size){
	void* stackFrsElement = malloc(sizeof(void*) * size);
	Stack* stack = malloc(sizeof(Stack));
	stack->stackFrsElement = stackFrsElement;
	stack->size = size;
	stack->stackPointer = -1;
	
	for(int i = 0; i < size; i++){
		stack->stackFrsElement[i] = NULL;
	}

	return stack;
}
void stackDelete(struct Stack* stack){
	for(int i = 0; i < stack->size; i++){
		if(stack->stackFrsElement[i] == NULL)
			continue;
		free(stack->stackFrsElement[i]);
	}
	free(stack->stackFrsElement);
	free(stack);
}
void stackPush(struct Stack* stack, void* element){
	if(stack->stackPointer >= stack->size){
		printf("%s. ","Stack struct is overflow");
		exit(-1);
	}
	stack->stackPointer++;
	(stack->stackFrsElement)[stack->stackPointer] = element;
} 
void* stackPop(struct Stack* stack){
	stack->stackPointer--;
	return stack->stackFrsElement[stack->stackPointer + 1];
} 