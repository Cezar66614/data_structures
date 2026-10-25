#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define ERROR_IMPL
#include "error.h" //[cite: 2]

#define T char*
#define SUFFIX str
#define STACK_IMPL
#include "data_structures/stack.h" //[cite: 1]

static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;
static int custom_free_calls = 0;

/* Helper to allocate memory for the data_free callback test */
char* my_strdup(const char* s) {
    if (!s) return NULL;
    char *d = malloc(strlen(s) + 1);
    if (d) strcpy(d, s);
    return d;
}

/* Mock free function tailored for dynamically allocated strings */
void mock_str_free(char* data) {
    if (data != NULL) {
        free(data);
    }
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

/* Custom assertion for string evaluation */
#define ASSERT_EQ_STR(actual, expected) do { \
    const char *_act = (actual); \
    const char *_exp = (expected); \
    if (_act == NULL || _exp == NULL) { \
        if (_act != _exp) { \
            fprintf(stderr, "\n  -> Failure at %s:%d: Expected '%s', got '%s'.", __FILE__, __LINE__, _exp ? _exp : "NULL", _act ? _act : "NULL"); \
            tests_failed++; \
            return; \
        } \
    } else if (strcmp(_act, _exp) != 0) { \
        fprintf(stderr, "\n  -> Failure at %s:%d: Expected '%s', got '%s'.", __FILE__, __LINE__, _exp, _act); \
        tests_failed++; \
        return; \
    } \
} while (0)

/* Edge Cases: Initialization */
void test_stack_init(void) {
    stack_t_str stack;
    
    // NULL stack parameter[cite: 3]
    ASSERT_EQ_INT(stack_init_str(NULL, NULL), ERR_PARAM_INVALID);

    // Valid initialization[cite: 3]
    ASSERT_EQ_INT(stack_init_str(&stack, mock_str_free), OK);
    ASSERT_EQ_PTR(stack.head, NULL);
    ASSERT_EQ_PTR(stack.tail, NULL);
    ASSERT_EQ_FNPTR(stack.data_free, mock_str_free);
}

/* Edge Cases: Push Operations */
void test_stack_push(void) {
    stack_t_str stack;
    stack_init_str(&stack, NULL);

    // Invalid parameters[cite: 3]
    ASSERT_EQ_INT(stack_push_str(NULL, "invalid"), ERR_PARAM_INVALID);

    // Push into empty stack[cite: 3]
    ASSERT_EQ_INT(stack_push_str(&stack, "first"), OK);
    ASSERT_EQ_PTR(stack.head, stack.tail);
    ASSERT_EQ_STR(stack.head->data, "first");
    ASSERT_EQ_PTR(stack.head->next, NULL);

    // Push into non-empty stack (Verifying LIFO structure)[cite: 3]
    ASSERT_EQ_INT(stack_push_str(&stack, "second"), OK);
    ASSERT_EQ_STR(stack.head->data, "second");           // New element becomes head (top of stack)[cite: 3]
    ASSERT_EQ_STR(stack.head->next->data, "first");      // Next element is the old head[cite: 3]
    ASSERT_EQ_STR(stack.tail->data, "first");            // Tail remains unchanged[cite: 3]

    stack_free_str(&stack);
}

/* Edge Cases: Peek Operations */
void test_stack_peek(void) {
    stack_t_str stack;
    char* data = NULL;
    
    stack_init_str(&stack, NULL);

    // Invalid parameters[cite: 3]
    ASSERT_EQ_INT(stack_peek_str(NULL, &data), ERR_PARAM_INVALID);
    ASSERT_EQ_INT(stack_peek_str(&stack, NULL), ERR_PARAM_INVALID);

    // Peek empty stack[cite: 3]
    ASSERT_EQ_INT(stack_peek_str(&stack, &data), ERR_EMPTY);

    // Peek populated stack[cite: 3]
    stack_push_str(&stack, "alpha");
    ASSERT_EQ_INT(stack_peek_str(&stack, &data), OK);
    ASSERT_EQ_STR(data, "alpha");

    // Ensure peek didn't modify the stack[cite: 3]
    ASSERT_EQ_STR(stack.head->data, "alpha");
    ASSERT_EQ_INT(stack_peek_str(&stack, &data), OK);

    stack_push_str(&stack, "beta");
    ASSERT_EQ_INT(stack_peek_str(&stack, &data), OK);
    ASSERT_EQ_STR(data, "beta"); // Should peek the newest element[cite: 3]

    stack_free_str(&stack);
}

/* Edge Cases: Pop Operations */
void test_stack_pop(void) {
    stack_t_str stack;
    char* data = NULL;
    
    stack_init_str(&stack, NULL);

    // Invalid parameters[cite: 3]
    ASSERT_EQ_INT(stack_pop_str(NULL, &data), ERR_PARAM_INVALID);
    ASSERT_EQ_INT(stack_pop_str(&stack, NULL), ERR_PARAM_INVALID);

    // Pop empty stack[cite: 3]
    ASSERT_EQ_INT(stack_pop_str(&stack, &data), ERR_EMPTY);

    // Populate stack: [Top: "top" -> "middle" -> "bottom" :Bottom][cite: 3]
    stack_push_str(&stack, "bottom");
    stack_push_str(&stack, "middle");
    stack_push_str(&stack, "top");

    // Multi-element LIFO pop validation[cite: 3]
    ASSERT_EQ_INT(stack_pop_str(&stack, &data), OK);
    ASSERT_EQ_STR(data, "top");
    ASSERT_EQ_STR(stack.head->data, "middle");

    ASSERT_EQ_INT(stack_pop_str(&stack, &data), OK);
    ASSERT_EQ_STR(data, "middle");
    ASSERT_EQ_STR(stack.head->data, "bottom");
    ASSERT_EQ_PTR(stack.head, stack.tail); // Only one element left[cite: 3]

    ASSERT_EQ_INT(stack_pop_str(&stack, &data), OK);
    ASSERT_EQ_STR(data, "bottom");
    ASSERT_EQ_PTR(stack.head, NULL);
    ASSERT_EQ_PTR(stack.tail, NULL);

    // Stack is empty again[cite: 3]
    ASSERT_EQ_INT(stack_pop_str(&stack, &data), ERR_EMPTY);

    stack_free_str(&stack);
}

/* Edge Cases: Custom data_free Callback and Cleanup */
void test_stack_free_and_callbacks(void) {
    stack_t_str stack;
    char* data = NULL;
    
    custom_free_calls = 0;
    stack_init_str(&stack, mock_str_free);

    ASSERT_EQ_INT(stack_free_str(NULL), ERR_PARAM_INVALID); //[cite: 3]

    // Allocate memory specifically so mock_str_free has valid heap memory to free
    stack_push_str(&stack, my_strdup("Node A"));
    stack_push_str(&stack, my_strdup("Node B"));
    stack_push_str(&stack, my_strdup("Node C"));

    // Trigger data_free via stack_pop (which internally calls sll_delete_first)[cite: 3]
    stack_pop_str(&stack, &data);
    ASSERT_EQ_INT(custom_free_calls, 1);

    // Trigger data_free via stack_free on remaining 2 nodes[cite: 3]
    ASSERT_EQ_INT(stack_free_str(&stack), OK);
    ASSERT_EQ_INT(custom_free_calls, 3);
    ASSERT_EQ_PTR(stack.head, NULL);
    ASSERT_EQ_PTR(stack.tail, NULL);

    // Freeing an already freed/empty stack[cite: 3]
    ASSERT_EQ_INT(stack_free_str(&stack), OK);
}

int main(void) {
    error_init(); //[cite: 2]

    printf("=== STACK CHAR* (STRING) TEST SUITE ===\n");
    RUN_TEST(test_stack_init);
    RUN_TEST(test_stack_push);
    RUN_TEST(test_stack_peek);
    RUN_TEST(test_stack_pop);
    RUN_TEST(test_stack_free_and_callbacks);

    printf("\nResults: %d/%d tests passed (%d failed).\n", 
           tests_passed, tests_run, tests_failed);

    error_free(); //[cite: 2]
    return (tests_failed == 0) ? 0 : 1;
}
