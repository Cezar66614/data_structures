#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define ERROR_IMPL
#include "error.h"

#define T int
#define SUFFIX int
#define STACK_IMPL
#include "data_structures/stack.h"

static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;
static int custom_free_calls = 0;

void mock_int_free(int data) {
    (void)data;
    custom_free_calls++;
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

#define ASSERT_EQ_FNPTR(actual, expected) do { \
    void (*_act)(void) = (void (*)(void))(actual); \
    void (*_exp)(void) = (void (*)(void))(expected); \
    if (_act != _exp) { \
        fprintf(stderr, "\n  -> Failure at %s:%d: Function pointer mismatch.", __FILE__, __LINE__); \
        tests_failed++; \
        return; \
    } \
} while (0)

/* Edge Cases: Initialization */
void test_stack_init(void) {
    stack_t_int stack;
    
    // NULL stack parameter
    ASSERT_EQ_INT(stack_init_int(NULL, NULL), ERR_PARAM_INVALID);

    // Valid initialization
    ASSERT_EQ_INT(stack_init_int(&stack, mock_int_free), OK);
    ASSERT_EQ_PTR(stack.head, NULL);
    ASSERT_EQ_PTR(stack.tail, NULL);
    ASSERT_EQ_FNPTR(stack.data_free, mock_int_free);
}

/* Edge Cases: Push Operations */
void test_stack_push(void) {
    stack_t_int stack;
    stack_init_int(&stack, NULL);

    // Invalid parameters
    ASSERT_EQ_INT(stack_push_int(NULL, 10), ERR_PARAM_INVALID);

    // Push into empty stack
    ASSERT_EQ_INT(stack_push_int(&stack, 10), OK);
    ASSERT_EQ_PTR(stack.head, stack.tail);
    ASSERT_EQ_INT(stack.head->data, 10);
    ASSERT_EQ_PTR(stack.head->next, NULL);

    // Push into non-empty stack (Verifying LIFO structure)
    ASSERT_EQ_INT(stack_push_int(&stack, 20), OK);
    ASSERT_EQ_INT(stack.head->data, 20);           // New element becomes head (top of stack)
    ASSERT_EQ_INT(stack.head->next->data, 10);     // Next element is the old head
    ASSERT_EQ_INT(stack.tail->data, 10);           // Tail remains unchanged

    stack_free_int(&stack);
}

/* Edge Cases: Peek Operations */
void test_stack_peek(void) {
    stack_t_int stack;
    int data = 0;
    
    stack_init_int(&stack, NULL);

    // Invalid parameters
    ASSERT_EQ_INT(stack_peek_int(NULL, &data), ERR_PARAM_INVALID);
    ASSERT_EQ_INT(stack_peek_int(&stack, NULL), ERR_PARAM_INVALID);

    // Peek empty stack
    ASSERT_EQ_INT(stack_peek_int(&stack, &data), ERR_EMPTY);

    // Peek populated stack
    stack_push_int(&stack, 42);
    ASSERT_EQ_INT(stack_peek_int(&stack, &data), OK);
    ASSERT_EQ_INT(data, 42);

    // Ensure peek didn't modify the stack
    ASSERT_EQ_INT(stack.head->data, 42);
    ASSERT_EQ_INT(stack_peek_int(&stack, &data), OK);

    stack_push_int(&stack, 99);
    ASSERT_EQ_INT(stack_peek_int(&stack, &data), OK);
    ASSERT_EQ_INT(data, 99); // Should peek the newest element

    stack_free_int(&stack);
}

/* Edge Cases: Pop Operations */
void test_stack_pop(void) {
    stack_t_int stack;
    int data = 0;
    
    stack_init_int(&stack, NULL);

    // Invalid parameters
    ASSERT_EQ_INT(stack_pop_int(NULL, &data), ERR_PARAM_INVALID);
    ASSERT_EQ_INT(stack_pop_int(&stack, NULL), ERR_PARAM_INVALID);

    // Pop empty stack
    ASSERT_EQ_INT(stack_pop_int(&stack, &data), ERR_EMPTY);

    // Populate stack: [Top: 30 -> 20 -> 10 :Bottom]
    stack_push_int(&stack, 10);
    stack_push_int(&stack, 20);
    stack_push_int(&stack, 30);

    // Multi-element LIFO pop validation
    ASSERT_EQ_INT(stack_pop_int(&stack, &data), OK);
    ASSERT_EQ_INT(data, 30);
    ASSERT_EQ_INT(stack.head->data, 20);

    ASSERT_EQ_INT(stack_pop_int(&stack, &data), OK);
    ASSERT_EQ_INT(data, 20);
    ASSERT_EQ_INT(stack.head->data, 10);
    ASSERT_EQ_PTR(stack.head, stack.tail); // Only one element left

    ASSERT_EQ_INT(stack_pop_int(&stack, &data), OK);
    ASSERT_EQ_INT(data, 10);
    ASSERT_EQ_PTR(stack.head, NULL);
    ASSERT_EQ_PTR(stack.tail, NULL);

    // Stack is empty again
    ASSERT_EQ_INT(stack_pop_int(&stack, &data), ERR_EMPTY);

    stack_free_int(&stack);
}

/* Edge Cases: Custom data_free Callback and Cleanup */
void test_stack_free_and_callbacks(void) {
    stack_t_int stack;
    int data = 0;
    
    custom_free_calls = 0;
    stack_init_int(&stack, mock_int_free);

    ASSERT_EQ_INT(stack_free_int(NULL), ERR_PARAM_INVALID);

    stack_push_int(&stack, 1);
    stack_push_int(&stack, 2);
    stack_push_int(&stack, 3);

    // Trigger data_free via stack_pop (which internally calls sll_delete_first)
    stack_pop_int(&stack, &data);
    ASSERT_EQ_INT(custom_free_calls, 1);

    // Trigger data_free via stack_free on remaining 2 nodes
    ASSERT_EQ_INT(stack_free_int(&stack), OK);
    ASSERT_EQ_INT(custom_free_calls, 3);
    ASSERT_EQ_PTR(stack.head, NULL);
    ASSERT_EQ_PTR(stack.tail, NULL);

    // Freeing an already freed/empty stack
    ASSERT_EQ_INT(stack_free_int(&stack), OK);
}

int main(void) {
    error_init();

    printf("=== STACK INT TEST SUITE ===\n");
    RUN_TEST(test_stack_init);
    RUN_TEST(test_stack_push);
    RUN_TEST(test_stack_peek);
    RUN_TEST(test_stack_pop);
    RUN_TEST(test_stack_free_and_callbacks);

    printf("\nResults: %d/%d tests passed (%d failed).\n", 
           tests_passed, tests_run, tests_failed);

    error_free();
    return (tests_failed == 0) ? 0 : 1;
}
