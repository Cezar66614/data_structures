#ifndef DLL_H
#define DLL_H

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


typedef struct GENERIC(_dll_node_struct) {
  T data;
  struct GENERIC(_dll_node_struct) *previous, *next;
} GENERIC(_dll_node_t);

typedef struct {
  GENERIC(_dll_node_t) *head, *tail;
  void (*data_free)(T data);
} GENERIC(dll_t);


error_codes GENERIC(dll_init)(GENERIC(dll_t) *list, void (*data_free)(T data));
error_codes GENERIC(dll_free)(GENERIC(dll_t) *list);

error_codes GENERIC(dll_insert_first)(GENERIC(dll_t) *list, T data);
error_codes GENERIC(dll_insert_last)(GENERIC(dll_t) *list, T data);
#ifndef NO_COMPARE
error_codes GENERIC(dll_insert_after)(GENERIC(dll_t) *list, T data, T search);
#endif // NO_COMPARE

error_codes GENERIC(dll_delete_first)(GENERIC(dll_t) *list);
error_codes GENERIC(dll_delete_last)(GENERIC(dll_t) *list);
#ifndef NO_COMPARE
error_codes GENERIC(dll_delete_value)(GENERIC(dll_t) *list, T search);
#endif // NO_COMPARE


#ifdef DLL_IMPL
#undef DLL_IMPL

#include <stdlib.h>


error_codes GENERIC(_dll_check)(GENERIC(dll_t) *list) {
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
GENERIC(_dll_node_t)* GENERIC(_dll_alloc_node)(T data) {
  GENERIC(_dll_node_t) *new;
  new = (GENERIC(_dll_node_t) *)malloc(sizeof(GENERIC(_dll_node_t)));
  if (new == NULL) {
    return NULL;
  }

  new->data = data;
  new->previous = new->next = NULL;

  return new;
}

error_codes GENERIC(dll_init)(GENERIC(dll_t) *list, void (*data_free)(T data)) {
  if (list == NULL) {
    return ERR_PARAM_INVALID;
  }

  list->head = list->tail = NULL;
  list->data_free = data_free;

  return OK;
}
error_codes GENERIC(dll_free)(GENERIC(dll_t) *list) {
  error_codes error_code = GENERIC(_dll_check)(list);
  if (error_code != OK) return error_code;

  if (list->head == NULL) return OK;

  GENERIC(_dll_node_t) *pointer, *next;
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

error_codes GENERIC(dll_insert_first)(GENERIC(dll_t) *list, T data) {
  error_codes error_code = GENERIC(_dll_check)(list);
  if (error_code != OK) return error_code;

  GENERIC(_dll_node_t) *new = GENERIC(_dll_alloc_node)(data);
  if (new == NULL) return ERR_MEM_ALLOC;

  new->next = list->head;
  if (list->head == NULL) {
    list->head = list->tail = new;
  } else {
    list->head->previous = new;
    list->head = new;
  }

  return OK;
}
error_codes GENERIC(dll_insert_last)(GENERIC(dll_t) *list, T data) {
  error_codes error_code = GENERIC(_dll_check)(list);
  if (error_code != OK) return error_code;

  GENERIC(_dll_node_t) *new = GENERIC(_dll_alloc_node)(data);
  if (new == NULL) return ERR_MEM_ALLOC;

  if (list->tail == NULL) {
    list->head = list->tail = new;
  } else {
    list->tail->next = new;
    new->previous = list->tail;
    list->tail = new;
  }

  return OK;
}
#ifndef NO_COMPARE
error_codes GENERIC(dll_insert_after)(GENERIC(dll_t) *list, T data, T search) {
  error_codes error_code = GENERIC(_dll_check)(list);
  if (error_code != OK) return error_code;

  GENERIC(_dll_node_t) *pointer;
  for (pointer = list->head; ((pointer != NULL) && (pointer->data != search)); pointer = pointer->next);
  if (pointer == NULL) {
    return ERR_SEARCH;
  }

  GENERIC(_dll_node_t) *new = GENERIC(_dll_alloc_node)(data);
  if (new == NULL) return ERR_MEM_ALLOC;

  new->next = pointer->next;
  if (pointer->next != NULL) {
    pointer->next->previous = new;
  }

  pointer->next = new;
  new->previous = pointer;

  if (list->tail == pointer) {
    list->tail = new;
  }

  return OK;
}
#endif // NO_COMPARE

error_codes GENERIC(dll_delete_first)(GENERIC(dll_t) *list) {
  error_codes error_code = GENERIC(_dll_check)(list);
  if (error_code != OK) return error_code;

  if (list->head == NULL) return OK;

  if (list->head == list->tail) {
    if (list->data_free) {
      list->data_free(list->head->data);
    }
    free(list->head);

    list->head = list->tail = NULL;

    return OK;
  }

  list->head = list->head->next;

  if (list->data_free) {
    list->data_free(list->head->previous->data);
  }
  free(list->head->previous);

  list->head->previous = NULL;

  return OK;
}
error_codes GENERIC(dll_delete_last)(GENERIC(dll_t) *list) {
  error_codes error_code = GENERIC(_dll_check)(list);
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

  list->tail = list->tail->previous;

  if (list->data_free) {
    list->data_free(list->tail->next->data);
  }
  free(list->tail->next);

  list->tail->next = NULL;

  return OK;
}
#ifndef NO_COMPARE
error_codes GENERIC(dll_delete_value)(GENERIC(dll_t) *list, T search) {
  error_codes error_code = GENERIC(_dll_check)(list);
  if (error_code != OK) return error_code;

  GENERIC(_dll_node_t) *pointer;
  for (pointer = list->head; ((pointer != NULL) && (pointer->data != search)); pointer = pointer->next);
  if (pointer == NULL) {
    return ERR_SEARCH;
  }

  if (pointer == list->head) {
    return GENERIC(dll_delete_first)(list);
  }
  if (pointer == list->tail) {
    return GENERIC(dll_delete_last)(list);
  }

  pointer->previous->next = pointer->next;
  pointer->next->previous = pointer->previous;

  if (list->data_free) {
    list->data_free(pointer->data);
  }
  free(pointer);

  return OK;
}
#endif // NO_COMPARE

#endif // DLL_IMPL


#undef GENERIC
#undef SUFFIX
#undef T
#undef CONCAT
#undef _CONCAT

#endif // DLL_H
