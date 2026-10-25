#ifndef SLL_H
#define SLL_H

#include "../error.h"


#ifndef T
#error Define 'T' before including the library.
#endif // T

#ifndef SUFFIX
#define SUFFIX T
#endif // SUFFIX

#define _CONCAT(A, B) A##_##B
#define CONCAT(A, B) _CONCAT(A, B)
#define GENERIC(N) CONCAT(N, SUFFIX)


typedef struct GENERIC(_sll_node_struct) {
  T data;
  struct GENERIC(_sll_node_struct) *next;
} GENERIC(_sll_node_t);

typedef struct {
  GENERIC(_sll_node_t) *head, *tail;
  void (*data_free)(T data);
} GENERIC(sll_t);


error_codes GENERIC(sll_init)(GENERIC(sll_t) *list, void (*data_free)(T data));
error_codes GENERIC(sll_free)(GENERIC(sll_t) *list);

error_codes GENERIC(sll_insert_first)(GENERIC(sll_t) *list, T data);
error_codes GENERIC(sll_insert_last)(GENERIC(sll_t) *list, T data);
error_codes GENERIC(sll_insert_after)(GENERIC(sll_t) *list, T data, T search);

error_codes GENERIC(sll_delete_first)(GENERIC(sll_t) *list);
error_codes GENERIC(sll_delete_last)(GENERIC(sll_t) *list);
error_codes GENERIC(sll_delete_value)(GENERIC(sll_t) *list, T search);


#ifdef SLL_IMPL
#undef SLL_IMPL

#include <stdlib.h>


error_codes GENERIC(_sll_check)(GENERIC(sll_t) *list) {
  if (list == NULL) {
    return ERR_PARAM_INVALID;
  }

  if (list->head == NULL) {
    if (list->tail == NULL) return OK;
    return ERR_PARAM_INVALID;
  }
  if (list->tail == NULL) {
    return ERR_PARAM_INVALID;
  }

  return OK;
}
GENERIC(_sll_node_t)* GENERIC(_sll_alloc_node)(T data) {
  GENERIC(_sll_node_t) *new;
  new = (GENERIC(_sll_node_t) *)malloc(sizeof(GENERIC(_sll_node_t)));
  if (new == NULL) {
    return NULL;
  }

  new->data = data;
  new->next = NULL;

  return new;
}

error_codes GENERIC(sll_init)(GENERIC(sll_t) *list, void (*data_free)(T data)) {
  if (list == NULL) {
    return ERR_PARAM_INVALID;
  }

  list->head = list->tail = NULL;
  list->data_free = data_free;

  return OK;
}
error_codes GENERIC(sll_free)(GENERIC(sll_t) *list) {
  error_codes error_code = GENERIC(_sll_check)(list);
  if (error_code != OK) return error_code;

  if (list->head == NULL) return OK;

  GENERIC(_sll_node_t) *pointer, *next;
  for (pointer = list->head; pointer != NULL; pointer = next) {
    next = pointer->next;

    if (list->data_free != NULL) {
      list->data_free(pointer->data);
    }
    free(pointer);
  }

  list->head = NULL;
  list->tail = NULL;

  return OK;
}

error_codes GENERIC(sll_insert_first)(GENERIC(sll_t) *list, T data) {
  error_codes error_code = GENERIC(_sll_check)(list);
  if (error_code != OK) return error_code;

  GENERIC(_sll_node_t) *new = GENERIC(_sll_alloc_node)(data);
  if (new == NULL) return ERR_MEM_ALLOC;

  new->next = list->head;
  if (list->head == NULL) {
    list->tail = new;
  }
  list->head = new;

  return OK;
}
error_codes GENERIC(sll_insert_last)(GENERIC(sll_t) *list, T data) {
  error_codes error_code = GENERIC(_sll_check)(list);
  if (error_code != OK) return error_code;

  GENERIC(_sll_node_t) *new = GENERIC(_sll_alloc_node)(data);
  if (new == NULL) return ERR_MEM_ALLOC;

  if (list->tail == NULL) {
    list->head = new;
  } else {
    list->tail->next = new;
  }
  list->tail = new;

  return OK;
}
error_codes GENERIC(sll_insert_after)(GENERIC(sll_t) *list, T data, T search) {
  error_codes error_code = GENERIC(_sll_check)(list);
  if (error_code != OK) return error_code;

  GENERIC(_sll_node_t) *pointer;
  for (pointer = list->head; ((pointer != NULL) && (pointer->data != search)); pointer = pointer->next);
  if (pointer == NULL) {
    return ERR_SEARCH;
  }

  GENERIC(_sll_node_t) *new = GENERIC(_sll_alloc_node)(data);
  if (new == NULL) return ERR_MEM_ALLOC;

  new->next = pointer->next;
  pointer->next = new;

  if (list->tail == pointer) {
    list->tail = new;
  }

  return OK;
}

error_codes GENERIC(sll_delete_first)(GENERIC(sll_t) *list) {
  error_codes error_code = GENERIC(_sll_check)(list);
  if (error_code != OK) return error_code;

  if (list->head == NULL) return OK;

  GENERIC(_sll_node_t) *old = list->head;
  list->head = list->head->next;
  if (list->tail == old) list->tail = list->head;

  if (list->data_free) {
    list->data_free(old->data);
  }
  free(old);

  return OK;
}
error_codes GENERIC(sll_delete_last)(GENERIC(sll_t) *list) {
  error_codes error_code = GENERIC(_sll_check)(list);
  if (error_code != OK) return error_code;

  if (list->tail == NULL) return OK;

  if (list->tail == list->head) {
    if (list->data_free) {
      list->data_free(list->tail->data);
    }
    free(list->tail);

    list->tail = list->head = NULL;

    return OK;
  }

  GENERIC(_sll_node_t) *previous;
  for (previous = list->head; ((previous != NULL) && (previous->next != list->tail)); previous = previous->next);
  if (previous == NULL) return ERR_PARAM_INVALID;

  previous->next = NULL;

  if (list->data_free) {
    list->data_free(list->tail->data);
  }
  free(list->tail);

  list->tail = previous;

  return OK;
}
error_codes GENERIC(sll_delete_value)(GENERIC(sll_t) *list, T search) {
  error_codes error_code = GENERIC(_sll_check)(list);
  if (error_code != OK) return error_code;

  GENERIC(_sll_node_t) *previous, *pointer;
  for (previous = NULL, pointer = list->head; ((pointer != NULL) && (pointer->data != search)); previous = pointer, pointer = pointer->next);
  if (pointer == NULL) {
    return ERR_SEARCH;
  }

  if (pointer == list->head) {
    return GENERIC(sll_delete_first)(list);
  }
  if (pointer == list->tail) {
    return GENERIC(sll_delete_last)(list);
  }

  previous->next = pointer->next;

  if (list->data_free) {
    list->data_free(pointer->data);
  }
  free(pointer);

  return OK;
}

#endif // SLL_IMPL


#undef GENERIC
#undef CONCAT
#undef _CONCAT
#ifndef INTERNAL
#undef SUFFIX
#undef T
#endif // INTERNAL

#endif // SLL_H
