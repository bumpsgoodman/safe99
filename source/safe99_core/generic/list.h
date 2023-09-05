#ifndef LIST_H
#define LIST_H

#include "safe99_common/defines.h"

// list는 링크 및 언링크만 해줌
// 실제 메모리 할당/해제는 사용자가 직접 해야 함

typedef struct list_node
{
    void* p_element;
    struct list_node* p_next;
    struct list_node* p_prev;
} list_node_t;

typedef struct list
{
    list_node_t* p_head;
    list_node_t* p_tail;
} list_t;

START_EXTERN_C

SAFE99_API void list_add_head(list_node_t** pp_head, list_node_t** pp_tail, list_node_t* p_node);

SAFE99_API void list_add_tail(list_node_t** pp_head, list_node_t** pp_tail, list_node_t* p_node);

SAFE99_API void list_delete_head(list_node_t** pp_head, list_node_t** pp_tail);

SAFE99_API void list_delete_tail(list_node_t** pp_head, list_node_t** pp_tail);

SAFE99_API void list_insert_node(list_node_t** pp_head, list_node_t** pp_tail, list_node_t* p_prev_node, list_node_t* p_node);

SAFE99_API void list_delete_node(list_node_t** pp_head, list_node_t** pp_tail, list_node_t* p_del_node);

END_EXTERN_C

#endif // LIST_H