#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define ERROR_IMPL
#include "error.h"

#define T char*
#define SUFFIX str
#define QUEUE_IMPL
#include "data_structures/queue.h"

static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;
static int custom_free_calls = 0;

void mock_str_free(char *data) {
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

#define ASSERT_EQ_STR(actual, expected) do { \
    const char *_act = (actual); \
    const char *_exp = (expected); \
    if (_act == NULL || _exp == NULL || strcmp(_act, _exp) != 0) { \
        fprintf(stderr, "\n  -> Failure at %s:%d: Expected '%s', got '%s'.", \
                __FILE__, __LINE__, _exp ? _exp : "NULL", _act ? _act : "NULL"); \
        tests_failed++; \
        return; \
    } \
} while (0)

/* Helper for duplicating strings portably */
char* my_strdup(const char* src) {
    if (!src) return NULL;
    char* dst = malloc(strlen(src) + 1);
    if (dst) strcpy(dst, src);
    return dst;
}

/* Edge Cases: Initialization */
void test_queue_init(void) {
    queue_t_str queue;
    
    ASSERT_EQ_INT(queue_init_str(NULL, NULL), ERR_PARAM_INVALID);

    ASSERT_EQ_INT(queue_init_str(&queue, mock_str_free), OK);
    ASSERT_EQ_PTR(queue.head, NULL);
    ASSERT_EQ_PTR(queue.tail, NULL);
    ASSERT_EQ_FNPTR(queue.data_free, mock_str_free);
}

/* Edge Cases: Enqueue Operations */
void test_queue_enqueue(void) {
    queue_t_str queue;
    queue_init_str(&queue, mock_str_free);

    ASSERT_EQ_INT(queue_enqueue_str(NULL, "invalid"), ERR_PARAM_INVALID);

    char *s1 = my_strdup("first");
    char *s2 = my_strdup("second");

    ASSERT_EQ_INT(queue_enqueue_str(&queue, s1), OK);
    ASSERT_EQ_PTR(queue.head, queue.tail);
    ASSERT_EQ_STR(queue.head->data, "first");
    
    ASSERT_EQ_INT(queue_enqueue_str(&queue, s2), OK);
    ASSERT_EQ_STR(queue.head->data, "first");
    ASSERT_EQ_STR(queue.head->next->data, "second");
    ASSERT_EQ_STR(queue.tail->data, "second");

    queue_free_str(&queue); // Automatically frees the strings
}

/* Edge Cases: Peek Operations */
void test_queue_peek(void) {
    queue_t_str queue;
    char *data = NULL;
    
    queue_init_str(&queue, mock_str_free);

    ASSERT_EQ_INT(queue_peek_str(NULL, &data), ERR_PARAM_INVALID);
    ASSERT_EQ_INT(queue_peek_str(&queue, NULL), ERR_PARAM_INVALID);

    ASSERT_EQ_INT(queue_peek_str(&queue, &data), ERR_EMPTY);

    char *s1 = my_strdup("alpha");
    char *s2 = my_strdup("beta");

    queue_enqueue_str(&queue, s1);
    ASSERT_EQ_INT(queue_peek_str(&queue, &data), OK);
    ASSERT_EQ_STR(data, "alpha");

    queue_enqueue_str(&queue, s2);
    ASSERT_EQ_INT(queue_peek_str(&queue, &data), OK);
    ASSERT_EQ_STR(data, "alpha"); // Always peeks the oldest string in a queue

    queue_free_str(&queue);
}

/* Edge Cases: Dequeue Operations 
 * (Disabled mock_str_free to safely test dequeued pointers without use-after-free) */
void test_queue_dequeue(void) {
    queue_t_str queue;
    char *data = NULL;
    
    // Explicitly initialize with NULL to manage dynamic strings manually here
    queue_init_str(&queue, NULL); 

    ASSERT_EQ_INT(queue_dequeue_str(NULL, &data), ERR_PARAM_INVALID);
    ASSERT_EQ_INT(queue_dequeue_str(&queue, NULL), ERR_PARAM_INVALID);

    ASSERT_EQ_INT(queue_dequeue_str(&queue, &data), ERR_EMPTY);

    char *s1 = my_strdup("apple");
    char *s2 = my_strdup("banana");
    char *s3 = my_strdup("cherry");

    queue_enqueue_str(&queue, s1);
    queue_enqueue_str(&queue, s2);
    queue_enqueue_str(&queue, s3);

    // Multi-element FIFO dequeue validation
    ASSERT_EQ_INT(queue_dequeue_str(&queue, &data), OK);
    ASSERT_EQ_STR(data, "apple");
    ASSERT_EQ_STR(queue.head->data, "banana");
    free(data); // Manually free

    ASSERT_EQ_INT(queue_dequeue_str(&queue, &data), OK);
    ASSERT_EQ_STR(data, "banana");
    ASSERT_EQ_STR(queue.head->data, "cherry");
    ASSERT_EQ_PTR(queue.head, queue.tail);
    free(data);

    ASSERT_EQ_INT(queue_dequeue_str(&queue, &data), OK);
    ASSERT_EQ_STR(data, "cherry");
    ASSERT_EQ_PTR(queue.head, NULL);
    ASSERT_EQ_PTR(queue.tail, NULL);
    free(data);

    ASSERT_EQ_INT(queue_dequeue_str(&queue, &data), ERR_EMPTY);

    queue_free_str(&queue);
}

/* Edge Cases: Custom data_free Callback and Cleanup */
void test_queue_free_and_callbacks(void) {
    queue_t_str queue;
    char *data = NULL;
    
    custom_free_calls = 0;
    queue_init_str(&queue, mock_str_free);

    ASSERT_EQ_INT(queue_free_str(NULL), ERR_PARAM_INVALID);

    queue_enqueue_str(&queue, my_strdup("dyn_1"));
    queue_enqueue_str(&queue, my_strdup("dyn_2"));
    queue_enqueue_str(&queue, my_strdup("dyn_3"));

    // Trigger data_free via queue_dequeue 
    // We ignore the dereferenced pointer since it's now freed memory!
    queue_dequeue_str(&queue, &data);
    ASSERT_EQ_INT(custom_free_calls, 1);

    // Trigger data_free via queue_free on remaining nodes
    ASSERT_EQ_INT(queue_free_str(&queue), OK);
    ASSERT_EQ_INT(custom_free_calls, 3);
    ASSERT_EQ_PTR(queue.head, NULL);
    ASSERT_EQ_PTR(queue.tail, NULL);

    ASSERT_EQ_INT(queue_free_str(&queue), OK);
}

int main(void) {
    error_init();

    printf("=== QUEUE STR TEST SUITE ===\n");
    RUN_TEST(test_queue_init);
    RUN_TEST(test_queue_enqueue);
    RUN_TEST(test_queue_peek);
    RUN_TEST(test_queue_dequeue);
    RUN_TEST(test_queue_free_and_callbacks);

    printf("\nResults: %d/%d tests passed (%d failed).\n", 
           tests_passed, tests_run, tests_failed);

    error_free();
    return (tests_failed == 0) ? 0 : 1;
}
