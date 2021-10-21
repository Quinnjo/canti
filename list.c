#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

void initList(LList* list) {
  list->head = NULL;
  list->tail = NULL;
  list->len = 0;
}

void freeList(LList* list) {
  LLNode* cur = list->head;
  LLNode* nxt;
  while(cur) {
    nxt = cur->next;
    free(cur->data);
    free(cur);
    cur = nxt;
  }
  initList(list);
}

void pushBackList(LList* list, void* data, int data_sz) {
  LLNode* new = (LLNode*)malloc(sizeof(LLNode));
  new->data = malloc(data_sz);
  memcpy(new->data, data, data_sz);
  new->next = NULL;
  if(list->len == 0) {
    list->head = new;
    list->tail = new;
  } else {
    list->tail->next = new;
    list->tail = new;
  }
  list->len += 1;
}

void* popBackList(LList* list) {
  assert(list->head && list->tail && list->len > 0);
  void* popped = list->tail->data;
  if(list->head == list->tail) {
    free(list->head);
    initList(list);
  } else{
    LLNode* pen = list->head;
    while(pen->next->next) {
      pen = pen->next;
    }
    free(list->tail);
    list->tail = pen;
    list->tail->next = NULL;
    list->len -= 1;
  }
  return popped;
}

void pushFrontList(LList* list, void* data, int data_sz) {
  LLNode* new = (LLNode*)malloc(sizeof(LLNode));
  new->data = malloc(data_sz);
  memcpy(new->data, data, data_sz);
  new->next = list->head;
  list->head = new;
  if(list->len == 0) {
    list->tail = new;
  }
  list->len += 1;
}

void* popFrontList(LList* list) {
  assert(list->head && list->tail && list->len > 0);
  void* popped = list->head->data;
  if(list->head == list->tail) {
    free(list->head);
    initList(list);
  } else {
    LLNode* sec = list->head->next;
    free(list->head);
    list->head = sec;
    list->len -= 1;
  }
  return popped;
}

void* peekBackList(LList* list) {
  assert(list->tail && list->len > 0);
  return list->tail->data;
}

void* peekFrontList(LList* list) {
  assert(list->head && list->len > 0);
  return list->head->data;
}

void* itemAtIndex(LList* list, int i) {
  assert(i >= 0 && i < list->len && list->head);
  int j = 0;
  LLNode* n = list->head;
  while(j != i) {
    n = n->next;
    j++;
  }
  return n->data;
}
    
int lengthList(LList* list) {
  return list->len;
}

void resetHead(LList* list, int i) {
  LLNode* cur = list->head;
  int c = 0;
  while(c < i) {
    if(cur == NULL) {
      initList(list);
      return;
    }
    free(cur->data);
    free(cur);
    c++;
  }
  list->head = cur;
}

LList* combineLLists(LList* L1, LList* L2) {
  assert(L1 && L2 && L1 != L2);
  LList* new = (LList*)malloc(sizeof(LList));

  /* if L1 is empty, return a shallow copy of L2 */
  if(L1->len == 0) {
    new->head = L2->head;
    new->tail = L2->tail;
    new->len = L2->len;
    free(L1);
    free(L2);
    return new;
  }
  initList(new);
  new->head = L1->head;
  new->tail = L2->tail;
  L1->tail->next = L2->head;
  new->len = L1->len + L2->len;
  free(L1);
  free(L2);
  return new;
}

void removeIndex(LList* list, int i) {
  assert(list && i >= 0 && i < list->len && list->len > 0);
  LLNode *prv, *cur;
  prv = NULL;
  cur = list->head;
  int j = 0;
  while(j != i) {
    prv = cur;
    cur = cur->next;   
    j++;
  }
  if(prv) {
    prv->next = cur->next;
  } else {
    list->head = cur->next;
  }
  free(cur->data);
  free(cur);
  list->len -= 1;

}

