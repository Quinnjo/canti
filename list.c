#include <assert.h>
#include "list.h"

void initList(LList* list, int data_sz) {
  list->head = NULL;
  list->tail = NULL;
  list->len = 0;
  list->data_sz = data_sz;
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

void pushBackList(LList* list, void* data) {
  LLNode* new = (LLNode*)malloc(sizeof(LLNode));
  new->data = data;
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

void pushFrontList(LList* list, void* data) {
  LLNode* new = (LLNode*)malloc(sizeof(LLNode));
  new->data = data;
  new->next = list->head;
  if(list->len == 0) {
    list->tail = new;
  }
  list->len += 1;
}

void* popFrontList(LList* list) {
  assert(list->head && list->tail && list->len > 0);
  void* popped = list->head->data;
  if(list->head == list->tail) {
    initList(list);
  } else {
    LLNode* sec = list->head->next;
    free(list->head);
    list->head = sec;
    list->len -= 1;
  }
  return popped;
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






















