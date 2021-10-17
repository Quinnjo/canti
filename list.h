#ifndef LIST_H
#define LIST_H

#include <stdlib.h>

typedef struct LLNode {
  struct LLNode* next;
  void* data;
} LLNode;

typedef struct LList {
  LLNode* head;
  LLNode* tail;
  int len;
  int data_sz;
} LList;

void initList(LList* list, int data_sz);
void freeList(LList* list);

void pushBackList(LList* list, void* data);
void* popBackList(LList* list);
void pushFrontList(LList* list, void* data);
void* popFrontList(LList* list);
void* peekBackList(LList* list);
void* peekFrontList(LList* list);

int lengthList(LList* list);
void* itemAtIndex(LList* list, int i);

/* 
 * set the head to the node at index i
 * in other words, cut off the first i elements
 */
void resetHead(LList* list, int i);

#endif
