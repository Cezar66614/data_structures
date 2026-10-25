#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#define ERROR_IMPL
#include "error.h"

typedef char * string;
#define T string
#define SUFFIX string
#define SLL_IMPL
#include "data_structures/sll.h"

static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;
static int string_free_calls = 0;

static string my_strdup(const char *s) {
    if (s == NULL) {
        return NULL;
    }
    size_t len = strlen(s) + 1;
    string dup = malloc(len);
    if (dup != NULL) {
        memcpy(dup, s, len);
    }
    return dup;
}

void custom_string_free(string str) {
    if (str != NULL) {
        string_free_calls++;
        free(str);
    }
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
    const void *_act = (const void *)(uintptr_t)(actual); \
    const void *_exp = (const void *)(uintptr_t)(expected); \
    if (_act != _exp) { \
        fprintf(stderr, "\n  -> Failure at %s:%d: Expected pointer %p, got %p.", __FILE__, __LINE__, _exp, _act); \
        tests_failed++; \
        return; \
    } \
} while (0)

#define ASSERT_EQ_STR(actual, expected) do { \
    const char *_act = (actual); \
    const char *_exp = (expected); \
    if (_act == NULL || _exp == NULL || strcmp(_act, _exp) != 0) { \
        fprintf(stderr, "\n  -> Failure at %s:%d: Expected string \"%s\", got \"%s\".", __FILE__, __LINE__, _exp ? _exp : "NULL", _act ? _act : "NULL"); \
        tests_failed++; \
        return; \
    } \
} while (0)

/* Edge Cases: Dynamic Memory Allocation and Invariants */
void test_string_init_and_check(void) {
    sll_string list;

    ASSERT_EQ_INT(sll_init_string(NULL, custom_string_free), ERR_PARAM_INVALID);
    ASSERT_EQ_INT(sll_init_string(&list, custom_string_free), OK);

    ASSERT_EQ_PTR(list.head, NULL);
    ASSERT_EQ_PTR(list.tail, NULL);
    ASSERT_EQ_PTR(list.data_free, custom_string_free);
    ASSERT_EQ_INT(_sll_check_string(&list), OK);
}

/* Edge Cases: Insertion and Pointer Equality Search */
void test_string_insert_and_search(void) {
    sll_string list;
    sll_init_string(&list, custom_string_free);

    string s1 = my_strdup("Alpha");
    string s2 = my_strdup("Beta");
    string s3 = my_strdup("Gamma");
    string not_in_list = my_strdup("Delta");

    // Insert last into empty list
    ASSERT_EQ_INT(sll_insert_last_string(&list, s1), OK);
    ASSERT_EQ_STR(list.head->data, "Alpha");
    ASSERT_EQ_PTR(list.head, list.tail);

    // Insert last
    ASSERT_EQ_INT(sll_insert_last_string(&list, s3), OK);
    ASSERT_EQ_STR(list.tail->data, "Gamma");

    // Insert after matching exact pointer (sll_insert_after checks pointer equality: pointer->data != search)
    ASSERT_EQ_INT(sll_insert_after_string(&list, s2, s1), OK); // insert s2 ("Beta") after s1 ("Alpha")
    ASSERT_EQ_STR(list.head->next->data, "Beta");

    // Attempt insert_after with non-existent pointer
    ASSERT_EQ_INT(sll_insert_after_string(&list, my_strdup("Epsilon"), not_in_list), ERR_SEARCH);

    free(not_in_list);
    sll_free_string(&list);
}

/* Edge Cases: Dynamic Memory Cleanup on Deletion */
void test_string_deletions_and_leaks(void) {
    sll_string list;
    string_free_calls = 0;
    sll_init_string(&list, custom_string_free);

    string s1 = my_strdup("Node1");
    string s2 = my_strdup("Node2");
    string s3 = my_strdup("Node3");

    sll_insert_last_string(&list, s1);
    sll_insert_last_string(&list, s2);
    sll_insert_last_string(&list, s3);

    // Delete head value
    ASSERT_EQ_INT(sll_delete_value_string(&list, s1), OK);
    ASSERT_EQ_INT(string_free_calls, 1);
    ASSERT_EQ_STR(list.head->data, "Node2");

    // Delete tail value
    ASSERT_EQ_INT(sll_delete_value_string(&list, s3), OK);
    ASSERT_EQ_INT(string_free_calls, 2);
    ASSERT_EQ_STR(list.tail->data, "Node2");

    // Delete remaining single element
    ASSERT_EQ_INT(sll_delete_first_string(&list), OK);
    ASSERT_EQ_INT(string_free_calls, 3);
    ASSERT_EQ_PTR(list.head, NULL);
    ASSERT_EQ_PTR(list.tail, NULL);

    sll_free_string(&list);
}

/* Edge Cases: NULL Data Insertion and Freeing */
void test_string_null_data_handling(void) {
    sll_string list;
    string_free_calls = 0;
    sll_init_string(&list, custom_string_free);

    // Inserting NULL pointer as data payload
    ASSERT_EQ_INT(sll_insert_last_string(&list, NULL), OK);
    ASSERT_EQ_PTR(list.head->data, NULL);

    // Delete first element with NULL payload
    ASSERT_EQ_INT(sll_delete_first_string(&list), OK);
    ASSERT_EQ_INT(string_free_calls, 0); // custom_string_free ignores NULL

    // Insert mixed payload and free list
    sll_insert_last_string(&list, my_strdup("ValidString"));
    sll_insert_last_string(&list, NULL);

    ASSERT_EQ_INT(sll_free_string(&list), OK);
    ASSERT_EQ_INT(string_free_calls, 1);
    ASSERT_EQ_PTR(list.head, NULL);
    ASSERT_EQ_PTR(list.tail, NULL);
}

int main(void) {
    error_init();

    printf("=== SLL STRING (char *) TEST SUITE ===\n");
    RUN_TEST(test_string_init_and_check);
    RUN_TEST(test_string_insert_and_search);
    RUN_TEST(test_string_deletions_and_leaks);
    RUN_TEST(test_string_null_data_handling);

    printf("\nResults: %d/%d tests passed (%d failed).\n", 
           tests_passed, tests_run, tests_failed);

    error_free();
    return (tests_failed == 0) ? 0 : 1;
}
