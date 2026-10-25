#ifndef QUEUE_H
#define QUEUE_H

#include "../error.h"


#ifndef T
#error Define 'T' before including the library.
#endif // T

#ifndef SUFFIX
#define SUFFIX T
#endif // SUFFIX

#define INTERNAL
#ifdef QUEUE_IMPL
#define SLL_IMPL
#endif // QUEUE_IMPL

#include "sll.h"

#define _CONCAT(A, B) A##_##B
#define CONCAT(A, B) _CONCAT(A, B)
#define GENERIC(N) CONCAT(N, SUFFIX)


typedef GENERIC(sll_t) GENERIC(queue_t);


error_codes GENERIC(queue_init)(GENERIC(queue_t) *queue, void (*data_free)(T data));
error_codes GENERIC(queue_free)(GENERIC(queue_t) *queue);

error_codes GENERIC(queue_enqueue)(GENERIC(queue_t) *queue, T data);
error_codes GENERIC(queue_dequeue)(GENERIC(queue_t) *queue, T *data);
error_codes GENERIC(queue_peek)(GENERIC(queue_t) *queue, T *data);

error_codes GENERIC(queue_isEmpty)(GENERIC(queue_t) *queue, bool *data);


#ifdef QUEUE_IMPL
#undef QUEUE_IMPL

error_codes GENERIC(queue_init)(GENERIC(queue_t) *queue, void (*data_free)(T data)) {
  return GENERIC(sll_init)(queue, data_free);
}
error_codes GENERIC(queue_free)(GENERIC(queue_t) *queue) {
  return GENERIC(sll_free)(queue);
}

error_codes GENERIC(queue_enqueue)(GENERIC(queue_t) *queue, T data) {
  return GENERIC(sll_insert_last)(queue, data);
}
error_codes GENERIC(queue_dequeue)(GENERIC(queue_t) *queue, T *data) {
  error_codes error_code = GENERIC(queue_peek)(queue, data);
  if (error_code != OK) return error_code;

  return GENERIC(sll_delete_first)(queue);
}
error_codes GENERIC(queue_peek)(GENERIC(queue_t) *queue, T *data) {
  error_codes error_code = GENERIC(_sll_check)(queue);
  if (error_code != OK) return error_code;

  if (data == NULL) {
    return ERR_PARAM_INVALID;
  }

  if (queue->head == NULL) {
    return ERR_EMPTY;
  }

  *data = queue->head->data;

  return OK;
}

error_codes GENERIC(queue_isEmpty)(GENERIC(queue_t) *queue, bool *data) {
  error_codes error_code = GENERIC(_sll_check)(queue);
  if (error_code != OK) return error_code;

  if (data == NULL) {
    return ERR_PARAM_INVALID;
  }

  *data = false;
  if (queue->head == NULL) *data = true;

  return OK;
}

#endif // QUEUE_IMPL


#undef GENERIC
#undef CONCAT
#undef _CONCAT
#undef INTERNAL
#undef SUFFIX
#undef T

#endif // QUEUE_H
