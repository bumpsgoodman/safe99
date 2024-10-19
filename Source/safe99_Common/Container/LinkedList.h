// 작성자: bumpsgoodman
// 작성일: 2024-08-21

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

typedef struct LINKED_LIST_NODE
{
    void*                       pElement;
    struct LINKED_LIST_NODE*    pNext;
    struct LINKED_LIST_NODE*    pPrev;
} LINKED_LIST_NODE;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void     __stdcall   LinkedListAddHead(LINKED_LIST_NODE** ppHead, LINKED_LIST_NODE** ppTail, LINKED_LIST_NODE* pNode);
void     __stdcall   LinkedListAddTail(LINKED_LIST_NODE** ppHead, LINKED_LIST_NODE** ppTail, LINKED_LIST_NODE* pNode);

void     __stdcall   LinkedListDeleteHead(LINKED_LIST_NODE** ppHead, LINKED_LIST_NODE** ppTail);
void     __stdcall   LinkedListDeleteTail(LINKED_LIST_NODE** ppHead, LINKED_LIST_NODE** ppTail);

void     __stdcall   LinkedListInsertNode(LINKED_LIST_NODE** ppHead, LINKED_LIST_NODE** ppTail, LINKED_LIST_NODE* pPrevNode, LINKED_LIST_NODE* pNode);
void     __stdcall   LinkedListDeleteNode(LINKED_LIST_NODE** ppHead, LINKED_LIST_NODE** ppTail, LINKED_LIST_NODE* pDeleteNode);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // LINKED_LIST_H