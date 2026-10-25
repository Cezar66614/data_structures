#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define ERROR_IMPL
#include "error.h" /*[cite: 2] */

#define T int
#define SUFFIX int
#define DLL_IMPL
#include "data_structures/dll.h" /*[cite: 3] */

static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;
static int custom_free_calls = 0;

void mock_int_free(int data) {
    (void)data;
    custom_free_calls++;
}

/* Test framework replicating sll_test_int.c[cite: 1] */
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

#define ASSERT_EQ_FNPTR(actual, expected) do { \
    void (*_act)(void) = (void (*)(void))(actual); \
    void (*_exp)(void) = (void (*)(void))(expected); \
    if (_act != _exp) { \
        fprintf(stderr, "\n  -> Failure at %s:%d: Function pointer mismatch.", __FILE__, __LINE__); \
        tests_failed++; \
        return; \
    } \
} while (0)

/* Edge Cases: Initialization and Invariants */
void test_init_and_check(void) {
    dll_t_int list;
    
    /* NULL list parameter[cite: 3] */
    ASSERT_EQ_INT(dll_init_int(NULL, NULL), ERR_PARAM_INVALID);
    ASSERT_EQ_INT(_dll_check_int(NULL), ERR_PARAM_INVALID);

    /* Valid initialization[cite: 3] */
    ASSERT_EQ_INT(dll_init_int(&list, mock_int_free), OK);
    ASSERT_EQ_PTR(list.head, NULL);
    ASSERT_EQ_PTR(list.tail, NULL);
    ASSERT_EQ_FNPTR(list.data_free, mock_int_free);
    ASSERT_EQ_INT(_dll_check_int(&list), OK);

    /* Corrupted state: head non-null, tail null[cite: 3] */
    _dll_node_t_int dummy = { .data = 10, .previous = NULL, .next = NULL };
    list.head = &dummy;
    list.tail = NULL;
    ASSERT_EQ_INT(_dll_check_int(&list), ERR_PARAM_INVALID);

    /* Corrupted state: head null, tail non-null[cite: 3] */
    list.head = NULL;
    list.tail = &dummy;
    ASSERT_EQ_INT(_dll_check_int(&list), ERR_PARAM_INVALID);
}

/* Edge Cases: Insertion Operations */
void test_insert_first_and_last(void) {
    dll_t_int list;
    dll_init_int(&list, NULL);

    /* Invalid parameters[cite: 3] */
    ASSERT_EQ_INT(dll_insert_first_int(NULL, 10), ERR_PARAM_INVALID);
    ASSERT_EQ_INT(dll_insert_last_int(NULL, 10), ERR_PARAM_INVALID);

    /* Insert last into empty list[cite: 3] */
    ASSERT_EQ_INT(dll_insert_last_int(&list, 10), OK);
    ASSERT_EQ_PTR(list.head, list.tail);
    ASSERT_EQ_INT(list.head->data, 10);
    ASSERT_EQ_PTR(list.head->previous, NULL);
    ASSERT_EQ_PTR(list.head->next, NULL);

    /* Insert last into non-empty list[cite: 3] */
    ASSERT_EQ_INT(dll_insert_last_int(&list, 20), OK);
    ASSERT_EQ_INT(list.head->data, 10);
    ASSERT_EQ_INT(list.tail->data, 20);
    ASSERT_EQ_PTR(list.head->next, list.tail);
    ASSERT_EQ_PTR(list.tail->previous, list.head);
    ASSERT_EQ_PTR(list.tail->next, NULL);

    /* Insert first into non-empty list[cite: 3] */
    ASSERT_EQ_INT(dll_insert_first_int(&list, 5), OK);
    ASSERT_EQ_INT(list.head->data, 5);
    ASSERT_EQ_INT(list.head->next->data, 10);
    ASSERT_EQ_PTR(list.head->next->previous, list.head);

    dll_free_int(&list);
}

void test_insert_after_edge_cases(void) {
    dll_t_int list;
    dll_init_int(&list, NULL);

    /* Insert after in empty list (Search target missing)[cite: 3] */
    ASSERT_EQ_INT(dll_insert_after_int(&list, 100, 10), ERR_SEARCH);

    /* Populate list: [10 <-> 30] */
    dll_insert_last_int(&list, 10);
    dll_insert_last_int(&list, 30);

    /* Insert after non-existent value[cite: 3] */
    ASSERT_EQ_INT(dll_insert_after_int(&list, 999, 50), ERR_SEARCH);

    /* Insert after head: insert 20 after 10 -> [10 <-> 20 <-> 30][cite: 3] */
    ASSERT_EQ_INT(dll_insert_after_int(&list, 20, 10), OK);
    ASSERT_EQ_INT(list.head->next->data, 20);
    ASSERT_EQ_PTR(list.head->next->previous, list.head);
    ASSERT_EQ_INT(list.head->next->next->data, 30);
    ASSERT_EQ_INT(list.tail->previous->data, 20);

    /* Insert after tail: insert 40 after 30 -> [10 <-> 20 <-> 30 <-> 40][cite: 3] */
    ASSERT_EQ_INT(dll_insert_after_int(&list, 40, 30), OK);
    ASSERT_EQ_INT(list.tail->data, 40);
    ASSERT_EQ_INT(list.tail->previous->data, 30);
    ASSERT_EQ_PTR(list.tail->next, NULL);

    dll_free_int(&list);
}

/* Edge Cases: Deletion Operations */
void test_delete_first_and_last(void) {
    dll_t_int list;
    dll_init_int(&list, NULL);

    /* NULL parameter[cite: 3] */
    ASSERT_EQ_INT(dll_delete_first_int(NULL), ERR_PARAM_INVALID);
    ASSERT_EQ_INT(dll_delete_last_int(NULL), ERR_PARAM_INVALID);

    /* Delete from empty list[cite: 3] */
    ASSERT_EQ_INT(dll_delete_first_int(&list), OK);
    ASSERT_EQ_INT(dll_delete_last_int(&list), OK);

    /* Single element deletion[cite: 3] */
    dll_insert_last_int(&list, 100);
    ASSERT_EQ_INT(dll_delete_last_int(&list), OK);
    ASSERT_EQ_PTR(list.head, NULL);
    ASSERT_EQ_PTR(list.tail, NULL);

    dll_insert_last_int(&list, 200);
    ASSERT_EQ_INT(dll_delete_first_int(&list), OK);
    ASSERT_EQ_PTR(list.head, NULL);
    ASSERT_EQ_PTR(list.tail, NULL);

    /* Multi-element sequence deletion */
    dll_insert_last_int(&list, 1);
    dll_insert_last_int(&list, 2);
    dll_insert_last_int(&list, 3);

    ASSERT_EQ_INT(dll_delete_first_int(&list), OK); /* removes 1 -> [2 <-> 3][cite: 3] */
    ASSERT_EQ_INT(list.head->data, 2);
    ASSERT_EQ_PTR(list.head->previous, NULL);

    ASSERT_EQ_INT(dll_delete_last_int(&list), OK);  /* removes 3 -> [2][cite: 3] */
    ASSERT_EQ_INT(list.tail->data, 2);
    ASSERT_EQ_PTR(list.tail->next, NULL);
    ASSERT_EQ_PTR(list.head, list.tail);

    dll_free_int(&list);
}

void test_delete_value_edge_cases(void) {
    dll_t_int list;
    dll_init_int(&list, NULL);

    /* Delete value in empty list[cite: 3] */
    ASSERT_EQ_INT(dll_delete_value_int(&list, 10), ERR_SEARCH);

    /* Populate list: [10 <-> 20 <-> 30 <-> 40] */
    dll_insert_last_int(&list, 10);
    dll_insert_last_int(&list, 20);
    dll_insert_last_int(&list, 30);
    dll_insert_last_int(&list, 40);

    /* Delete non-existent value[cite: 3] */
    ASSERT_EQ_INT(dll_delete_value_int(&list, 99), ERR_SEARCH);

    /* Delete head value (10)[cite: 3] */
    ASSERT_EQ_INT(dll_delete_value_int(&list, 10), OK);
    ASSERT_EQ_INT(list.head->data, 20);
    ASSERT_EQ_PTR(list.head->previous, NULL);

    /* Delete tail value (40)[cite: 3] */
    ASSERT_EQ_INT(dll_delete_value_int(&list, 40), OK);
    ASSERT_EQ_INT(list.tail->data, 30);
    ASSERT_EQ_PTR(list.tail->next, NULL);

    /* Delete middle value (20) -> remains [30][cite: 3] */
    ASSERT_EQ_INT(dll_delete_value_int(&list, 20), OK);
    ASSERT_EQ_INT(list.head->data, 30);
    ASSERT_EQ_PTR(list.head, list.tail);

    dll_free_int(&list);
}

/* Edge Cases: Custom data_free Callback and Cleanup */
void test_free_and_callbacks(void) {
    dll_t_int list;
    custom_free_calls = 0;
    dll_init_int(&list, mock_int_free);

    ASSERT_EQ_INT(dll_free_int(NULL), ERR_PARAM_INVALID); /*[cite: 3] */

    dll_insert_last_int(&list, 1);
    dll_insert_last_int(&list, 2);
    dll_insert_last_int(&list, 3);

    /* Trigger data_free via dll_delete_first[cite: 3] */
    dll_delete_first_int(&list);
    ASSERT_EQ_INT(custom_free_calls, 1);

    /* Trigger data_free via dll_delete_value[cite: 3] */
    dll_delete_value_int(&list, 2);
    ASSERT_EQ_INT(custom_free_calls, 2);

    /* Trigger data_free via dll_free on remaining 1 node[cite: 3] */
    ASSERT_EQ_INT(dll_free_int(&list), OK);
    ASSERT_EQ_INT(custom_free_calls, 3);
    ASSERT_EQ_PTR(list.head, NULL);
    ASSERT_EQ_PTR(list.tail, NULL);

    /* Freeing an already freed/empty list[cite: 3] */
    ASSERT_EQ_INT(dll_free_int(&list), OK);
}

int main(void) {
    error_init(); /*[cite: 2] */

    printf("=== DLL INT TEST SUITE ===\n");
    RUN_TEST(test_init_and_check);
    RUN_TEST(test_insert_first_and_last);
    RUN_TEST(test_insert_after_edge_cases);
    RUN_TEST(test_delete_first_and_last);
    RUN_TEST(test_delete_value_edge_cases);
    RUN_TEST(test_free_and_callbacks);

    printf("\nResults: %d/%d tests passed (%d failed).\n", 
           tests_passed, tests_run, tests_failed);

    error_free(); /*[cite: 2] */
    return (tests_failed == 0) ? 0 : 1;
}
