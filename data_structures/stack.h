#ifndef STACK_H
#define STACK_H

#include "../error.h"


#ifndef T
#error Define 'T' before including the library.
#endif // T

#ifndef SUFFIX
#define SUFFIX T
#endif // SUFFIX

#define INTERNAL
#ifdef STACK_IMPL
#define SLL_IMPL
#endif // STACK_IMPL

#include "sll.h"

#define _CONCAT(A, B) A##_##B
#define CONCAT(A, B) _CONCAT(A, B)
#define GENERIC(N) CONCAT(N, SUFFIX)


typedef GENERIC(sll_t) GENERIC(stack_t);


error_codes GENERIC(stack_init)(GENERIC(stack_t) *stack, void (*data_free)(T data));
error_codes GENERIC(stack_free)(GENERIC(stack_t) *stack);

error_codes GENERIC(stack_push)(GENERIC(stack_t) *stack, T data);
error_codes GENERIC(stack_pop)(GENERIC(stack_t) *stack, T *data);
error_codes GENERIC(stack_peek)(GENERIC(stack_t) *stack, T *data);

error_codes GENERIC(stack_isEmpty)(GENERIC(stack_t) *stack, bool *data);


#ifdef STACK_IMPL
#undef STACK_IMPL

error_codes GENERIC(stack_init)(GENERIC(stack_t) *stack, void (*data_free)(T data)) {
  return GENERIC(sll_init)(stack, data_free);
}
error_codes GENERIC(stack_free)(GENERIC(stack_t) *stack) {
  return GENERIC(sll_free)(stack);
}

error_codes GENERIC(stack_push)(GENERIC(stack_t) *stack, T data) {
  return GENERIC(sll_insert_first)(stack, data);
}
error_codes GENERIC(stack_pop)(GENERIC(stack_t) *stack, T *data) {
  error_codes error_code = GENERIC(stack_peek)(stack, data);
  if (error_code != OK) return error_code;

  return GENERIC(sll_delete_first)(stack);
}
error_codes GENERIC(stack_peek)(GENERIC(stack_t) *stack, T *data) {
  error_codes error_code = GENERIC(_sll_check)(stack);
  if (error_code != OK) return error_code;

  if (data == NULL) {
    return ERR_PARAM_INVALID;
  }

  if (stack->head == NULL) {
    return ERR_EMPTY;
  }

  *data = stack->head->data;

  return OK;
}

error_codes GENERIC(stack_isEmpty)(GENERIC(stack_t) *stack, bool *data) {
  error_codes error_code = GENERIC(_sll_check)(stack);
  if (error_code != OK) return error_code;

  if (data == NULL) {
    return ERR_PARAM_INVALID;
  }

  *data = false;
  if (stack->head == NULL) *data = true;

  return OK;
}

#endif // STACK_IMPL


#undef GENERIC
#undef CONCAT
#undef _CONCAT
#undef INTERNAL
#undef SUFFIX
#undef T

#endif // STACK_H
