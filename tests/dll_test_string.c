#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define ERROR_IMPL
#include "error.h" /*[cite: 2] */

/* Explicitly setting SUFFIX avoids invalid macros like dll_init_char*[cite: 3] */
#define T char*
#define SUFFIX str
#define DLL_IMPL
#include "data_structures/dll.h" /*[cite: 3] */

static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;
static int custom_free_calls = 0;

/* Mocks free for dynamically allocated strings and tracks metrics[cite: 1, 3] */
void mock_str_free(char* data) {
    if (data) {
        custom_free_calls++;
        free(data);
    }
}

/* Helper to allocate strings dynamically */
char* my_strdup(const char* s) {
    if (!s) return NULL;
    char* d = malloc(strlen(s) + 1);
    if (d) strcpy(d, s);
    return d;
}

#define RUN_TEST(test) do { \
    printf("Running %-35s ... ", #test); \
    int prev_failed = tests_failed; \
    test(); \
    tests_run++; \
    if (tests_failed == prev_failed) { \
        tests_passed++; \
        printf("[PASS]\n"); \
    } else { \
        printf("[FAIL]\n"); \
    } \
} while (0)

#define ASSERT_TRUE(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "\n  -> Failure at %s:%d: Assertion '%s' failed.", __FILE__, __LINE__, #cond); \
        tests_failed++; \
        return; \
    } \
} while (0)

#define ASSERT_EQ_INT(actual, expected) do { \
    int _act = (int)(actual); \
    int _exp = (int)(expected); \
    if (_act != _exp) { \
        fprintf(stderr, "\n  -> Failure at %s:%d: Expected %d, got %d.", __FILE__, __LINE__, _exp, _act); \
        tests_failed++; \
        return; \
    } \
} while (0)

#define ASSERT_EQ_PTR(actual, expected) do { \
    const void *_act = (actual); \
    const void *_exp = (expected); \
    if (_act != _exp) { \
        fprintf(stderr, "\n  -> Failure at %s:%d: Expected pointer %p, got %p.", __FILE__, __LINE__, _exp, _act); \
        tests_failed++; \
        return; \
    } \
} while (0)

#define ASSERT_EQ_STR(actual, expected) do { \
    const char *_act = (actual); \
    const char *_exp = (expected); \
    if (!_act || !_exp || strcmp(_act, _exp) != 0) { \
        fprintf(stderr, "\n  -> Failure at %s:%d: Expected string '%s', got '%s'.", __FILE__, __LINE__, _exp ? _exp : "NULL", _act ? _act : "NULL"); \
        tests_failed++; \
        return; \
    } \
} while (0)

/* Edge Cases: Initialization and Invariants */
void test_init_and_check_str(void) {
    dll_t_str list;
    
    ASSERT_EQ_INT(dll_init_str(NULL, NULL), ERR_PARAM_INVALID); /*[cite: 3] */
    
    ASSERT_EQ_INT(dll_init_str(&list, mock_str_free), OK); /*[cite: 3] */
    ASSERT_EQ_PTR(list.head, NULL);
    ASSERT_EQ_PTR(list.tail, NULL);
    
    /* Simulate corruption[cite: 3] */
    _dll_node_t_str dummy = { .data = "dummy", .previous = NULL, .next = NULL };
    list.head = &dummy;
    ASSERT_EQ_INT(_dll_check_str(&list), ERR_PARAM_INVALID);
}

/* Edge Cases: Insertion Operations */
void test_insert_first_and_last_str(void) {
    dll_t_str list;
    dll_init_str(&list, mock_str_free);

    char *s1 = my_strdup("first");
    char *s2 = my_strdup("last");

    ASSERT_EQ_INT(dll_insert_first_str(&list, s1), OK); /*[cite: 3] */
    ASSERT_EQ_PTR(list.head, list.tail);
    ASSERT_EQ_STR(list.head->data, "first");

    ASSERT_EQ_INT(dll_insert_last_str(&list, s2), OK); /*[cite: 3] */
    ASSERT_EQ_STR(list.tail->data, "last");
    ASSERT_EQ_PTR(list.tail->previous, list.head);

    dll_free_str(&list); /*[cite: 3] */
}

/* Note: dll_insert_after matches by pointer identity (pointer->data != search), not strcmp[cite: 3] */
void test_insert_after_edge_cases_str(void) {
    dll_t_str list;
    dll_init_str(&list, mock_str_free);

    char *s1 = my_strdup("apple");
    char *s2 = my_strdup("banana");
    char *s3 = my_strdup("cherry");
    char *s_missing = "missing";

    dll_insert_last_str(&list, s1);
    dll_insert_last_str(&list, s3);

    /* Search for missing pointer[cite: 3] */
    ASSERT_EQ_INT(dll_insert_after_str(&list, s2, s_missing), ERR_SEARCH);

    /* Insert banana after apple -> [apple <-> banana <-> cherry][cite: 3] */
    ASSERT_EQ_INT(dll_insert_after_str(&list, s2, s1), OK);
    ASSERT_EQ_STR(list.head->next->data, "banana");
    ASSERT_EQ_PTR(list.head->next->previous, list.head);
    ASSERT_EQ_PTR(list.head->next->next->data, s3);

    dll_free_str(&list); /*[cite: 3] */
}

/* Edge Cases: Deletion Operations */
void test_delete_first_and_last_str(void) {
    dll_t_str list;
    dll_init_str(&list, mock_str_free);

    char *s1 = my_strdup("A");
    char *s2 = my_strdup("B");
    
    dll_insert_last_str(&list, s1);
    dll_insert_last_str(&list, s2);

    /* Deletes "A", frees memory[cite: 3] */
    ASSERT_EQ_INT(dll_delete_first_str(&list), OK); 
    ASSERT_EQ_STR(list.head->data, "B");
    ASSERT_EQ_PTR(list.head->previous, NULL);

    /* Deletes "B", frees memory[cite: 3] */
    ASSERT_EQ_INT(dll_delete_last_str(&list), OK);
    ASSERT_EQ_PTR(list.head, NULL);
    ASSERT_EQ_PTR(list.tail, NULL);

    dll_free_str(&list); /*[cite: 3] */
}

/* Edge Cases: Custom data_free Callback and Cleanup on strings */
void test_free_and_callbacks_str(void) {
    dll_t_str list;
    custom_free_calls = 0;
    dll_init_str(&list, mock_str_free); /*[cite: 3] */

    char *s1 = my_strdup("leak1");
    char *s2 = my_strdup("leak2");
    char *s3 = my_strdup("leak3");

    dll_insert_last_str(&list, s1);
    dll_insert_last_str(&list, s2);
    dll_insert_last_str(&list, s3);

    /* Deleting middle string by exact pointer reference[cite: 3] */
    ASSERT_EQ_INT(dll_delete_value_str(&list, s2), OK);
    ASSERT_EQ_INT(custom_free_calls, 1);
    ASSERT_EQ_STR(list.head->next->data, "leak3");
    ASSERT_EQ_PTR(list.tail->previous, list.head);

    /* Freeing remaining 2 strings[cite: 3] */
    ASSERT_EQ_INT(dll_free_str(&list), OK);
    ASSERT_EQ_INT(custom_free_calls, 3);
}

int main(void) {
    error_init(); /*[cite: 2] */

    printf("=== DLL STR (DYNAMIC) TEST SUITE ===\n");
    RUN_TEST(test_init_and_check_str);
    RUN_TEST(test_insert_first_and_last_str);
    RUN_TEST(test_insert_after_edge_cases_str);
    RUN_TEST(test_delete_first_and_last_str);
    RUN_TEST(test_free_and_callbacks_str);

    printf("\nResults: %d/%d tests passed (%d failed).\n", 
           tests_passed, tests_run, tests_failed);

    error_free(); /*[cite: 2] */
    return (tests_failed == 0) ? 0 : 1;
}
