// 작성자: bumpsgoodman
// 작성일: 2024-08-21

#include "Precompiled.h"
#include "LinkedList.h"
#include "../Common.h"

void __stdcall LinkedListAddHead(LINKED_LIST_NODE** ppHead, LINKED_LIST_NODE** ppTail, LINKED_LIST_NODE* pNewNode)
{
    ASSERT(ppHead != NULL, "ppHead is NULL");
    ASSERT(ppTail != NULL, "ppTail is NULL");
    ASSERT(pNewNode != NULL, "pNewNode is NULL");

    if (*ppHead == nullptr)
    {
        *ppHead = *ppTail = pNewNode;
        pNewNode->pNext = nullptr;
        pNewNode->pPrev = nullptr;
    }
    else
    {
        // 이미 추가된 노드거나 널 포인터를 추가할 경우
        ASSERT(*ppHead != pNewNode, "pNewNode same as head node or NULL");

        pNewNode->pNext = *ppHead;
        pNewNode->pPrev = nullptr;

        (*ppHead)->pPrev = pNewNode;
        *ppHead = pNewNode;
    }
}

void __stdcall LinkedListAddTail(LINKED_LIST_NODE** ppHead, LINKED_LIST_NODE** ppTail, LINKED_LIST_NODE* pNewNode)
{
    ASSERT(ppHead != NULL, "ppHead is NULL");
    ASSERT(ppTail != NULL, "ppTail is NULL");
    ASSERT(pNewNode != NULL, "pNewNode is NULL");

    if (*ppTail == nullptr)
    {
        *ppTail = *ppHead = pNewNode;
        pNewNode->pNext = nullptr;
        pNewNode->pPrev = nullptr;
    }
    else
    {
        // 이미 추가된 노드거나 널 포인터를 추가할 경우
        ASSERT(*ppHead != pNewNode, "pNewNode same as tail node or NULL");

        pNewNode->pNext = nullptr;
        pNewNode->pPrev = *ppTail;

        (*ppTail)->pNext = pNewNode;
        *ppTail = pNewNode;
    }
}

void __stdcall LinkedListDeleteHead(LINKED_LIST_NODE** ppHead, LINKED_LIST_NODE** ppTail)
{
    ASSERT(ppHead != NULL, "ppHead is NULL");
    ASSERT(ppTail != NULL, "ppTail is NULL");

    *ppHead = (*ppHead)->pNext;
    if (*ppHead == nullptr)
    {
        *ppTail = nullptr;
    }
    else
    {
        (*ppHead)->pPrev = nullptr;
    }
}

void __stdcall LinkedListDeleteTail(LINKED_LIST_NODE** ppHead, LINKED_LIST_NODE** ppTail)
{
    ASSERT(ppHead != NULL, "ppHead is NULL");
    ASSERT(ppTail != NULL, "ppTail is NULL");

    *ppTail = (*ppTail)->pPrev;
    if (*ppTail == nullptr)
    {
        *ppHead = nullptr;
    }
    else
    {
        (*ppTail)->pNext = nullptr;
    }
}

void __stdcall LinkedListInsertNode(LINKED_LIST_NODE** ppHead, LINKED_LIST_NODE** ppTail, LINKED_LIST_NODE* prevNode, LINKED_LIST_NODE* pNewNode)
{
    ASSERT(ppHead != NULL, "ppHead is NULL");
    ASSERT(ppTail != NULL, "ppTail is NULL");
    ASSERT(prevNode != NULL, "prevNode is NULL");
    ASSERT(pNewNode != NULL, "pNewNode is NULL");

    if (*ppHead == prevNode)
    {
        LinkedListAddHead(ppHead, ppTail, pNewNode);
    }
    else if (*ppTail == prevNode)
    {
        LinkedListAddTail(ppHead, ppTail, pNewNode);
    }
    else
    {
        pNewNode->pNext = prevNode;
        pNewNode->pPrev = prevNode->pPrev;
        prevNode->pPrev->pNext = pNewNode;
        prevNode->pPrev = pNewNode;
    }
}

void __stdcall LinkedListDeleteNode(LINKED_LIST_NODE** ppHead, LINKED_LIST_NODE** ppTail, LINKED_LIST_NODE* pDeleteNode)
{
    ASSERT(ppHead != NULL, "ppHead is NULL");
    ASSERT(ppTail != NULL, "ppTail is NULL");
    ASSERT(pDeleteNode != NULL, "pDeleteNode is NULL");

    if (*ppHead == pDeleteNode)
    {
        LinkedListDeleteHead(ppHead, ppTail);
    }
    else if (*ppTail == pDeleteNode)
    {
        LinkedListDeleteTail(ppHead, ppTail);
    }
    else
    {
        pDeleteNode->pPrev->pNext = pDeleteNode->pNext;
        pDeleteNode->pNext->pPrev = pDeleteNode->pPrev;

        pDeleteNode->pPrev = nullptr;
        pDeleteNode->pNext = nullptr;
    }
}