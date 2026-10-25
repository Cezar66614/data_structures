#ifndef ERROR_H
#define ERROR_H

typedef enum {
  OK = 0,
  ERR_MEM_ALLOC,
  ERR_PARAM_INVALID,
  ERR_SEARCH,
  ERR_EMPTY,
} error_codes;

void error_init(void);
void error_free(void);

const char *error_string(error_codes error_code);

#ifdef ERROR_IMPL
#undef ERROR_IMPL

#include <string.h>

#define ERROR_CODE_STRING_LENGTH 256 - 1
char _error_codes_strings[8][ERROR_CODE_STRING_LENGTH + 1];

void error_init(void) {
  strncpy(_error_codes_strings[OK], "No error.\0", ERROR_CODE_STRING_LENGTH);
  _error_codes_strings[OK][ERROR_CODE_STRING_LENGTH] = '\0';

  strncpy(_error_codes_strings[ERR_MEM_ALLOC], "ERROR: Failed to allocate memory.\0", ERROR_CODE_STRING_LENGTH);
  _error_codes_strings[ERR_MEM_ALLOC][ERROR_CODE_STRING_LENGTH] = '\0';

  strncpy(_error_codes_strings[ERR_PARAM_INVALID], "ERROR: Given parameter is invalid.\0", ERROR_CODE_STRING_LENGTH);
  _error_codes_strings[ERR_PARAM_INVALID][ERROR_CODE_STRING_LENGTH] = '\0';

  strncpy(_error_codes_strings[ERR_SEARCH], "ERROR: The search failed.\0", ERROR_CODE_STRING_LENGTH);
  _error_codes_strings[ERR_SEARCH][ERROR_CODE_STRING_LENGTH] = '\0';

  strncpy(_error_codes_strings[ERR_EMPTY], "ERROR: The data structure is empty.\0", ERROR_CODE_STRING_LENGTH);
  _error_codes_strings[ERR_EMPTY][ERROR_CODE_STRING_LENGTH] = '\0';

  return;
}
void error_free(void) {
  return;
}

const char *error_string(error_codes error_code) {
  return _error_codes_strings[error_code];
}

#endif // ERROR_IMPL

#endif // ERROR_H
