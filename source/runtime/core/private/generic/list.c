#include <stdbool.h>

#include "generic/list.h"
#include "util/assert.h"

void list_add_head(list_node_t** pp_head, list_node_t** pp_tail, list_node_t* p_node)
{
    ASSERT(pp_head != NULL, "pp_head == NULL");
    ASSERT(pp_tail != NULL, "pp_tail == NULL");
    ASSERT(p_node != NULL, "p_node == NULL");

    if (*pp_head == NULL)
    {
        *pp_head = *pp_tail = p_node;
        p_node->p_next = NULL;
        p_node->p_prev = NULL;
    }
    else
    {
        p_node->p_next = *pp_head;
        p_node->p_prev = NULL;

        (*pp_head)->p_prev = p_node;
        *pp_head = p_node;
    }
}

void list_add_tail(list_node_t** pp_head, list_node_t** pp_tail, list_node_t* p_node)
{
    ASSERT(pp_head != NULL, "pp_head == NULL");
    ASSERT(pp_tail != NULL, "pp_tail == NULL");
    ASSERT(p_node != NULL, "p_node == NULL");

    if (*pp_tail == NULL)
    {
        *pp_tail = *pp_head = p_node;
        p_node->p_next = NULL;
        p_node->p_prev = NULL;
    }
    else
    {
        p_node->p_next = NULL;
        p_node->p_prev = *pp_tail;

        (*pp_tail)->p_next = p_node;
        *pp_tail = p_node;
    }
}

void list_delete_head(list_node_t** pp_head, list_node_t** pp_tail)
{
    ASSERT(pp_head != NULL, "pp_head == NULL");
    ASSERT(pp_tail != NULL, "pp_tail == NULL");

    if (*pp_head == NULL)
    {
        ASSERT(false, "*pp_head == NULL");
        return;
    }

    *pp_head = (*pp_head)->p_next;
    if (*pp_head == NULL)
    {
        *pp_tail = NULL;
    }
    else
    {
        (*pp_head)->p_prev = NULL;
    }
}

void list_delete_tail(list_node_t** pp_head, list_node_t** pp_tail)
{
    ASSERT(pp_head != NULL, "pp_head == NULL");
    ASSERT(pp_tail != NULL, "pp_tail == NULL");

    if (*pp_tail == NULL)
    {
        ASSERT(false, "*pp_tail == NULL");
        return;
    }

    *pp_tail = (*pp_tail)->p_prev;
    if (*pp_tail == NULL)
    {
        *pp_head = NULL;
    }
    else
    {
        (*pp_tail)->p_next = NULL;
    }
}

void list_insert_node(list_node_t** pp_head, list_node_t** pp_tail,
                      list_node_t* p_prev_node, list_node_t* p_node)
{
    ASSERT(pp_head != NULL, "pp_head == NULL");
    ASSERT(pp_tail != NULL, "pp_tail == NULL");
    ASSERT(p_prev_node != NULL, "p_del_node == NULL");
    ASSERT(p_node != NULL, "p_node == NULL");

    if (*pp_head == p_prev_node)
    {
        list_add_head(pp_head, pp_tail, p_node);
    }
    else if (*pp_tail == p_prev_node)
    {
        list_add_tail(pp_head, pp_tail, p_node);
    }
    else
    {
        p_node->p_next = p_prev_node;
        p_node->p_prev = p_prev_node->p_prev;
        p_prev_node->p_prev->p_next = p_node;
        p_prev_node->p_prev = p_node;
    }
}

void list_delete_node(list_node_t** pp_head, list_node_t** pp_tail, list_node_t* p_del_node)
{
    ASSERT(pp_head != NULL, "pp_head == NULL");
    ASSERT(pp_tail != NULL, "pp_tail == NULL");
    ASSERT(p_del_node != NULL, "p_del_node == NULL");

    if (*pp_head == p_del_node)
    {
        list_delete_head(pp_head, pp_tail);
    }
    else if (*pp_tail == p_del_node)
    {
        list_delete_tail(pp_head, pp_tail);
    }
    else
    {
        p_del_node->p_prev->p_next = p_del_node->p_next;
        p_del_node->p_next->p_prev = p_del_node->p_prev;

        p_del_node->p_prev = NULL;
        p_del_node->p_next = NULL;
    }
}