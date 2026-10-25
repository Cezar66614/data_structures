#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define ERROR_IMPL
#include "error.h"

#define T int
#define SUFFIX int
#define QUEUE_IMPL
#include "data_structures/queue.h"

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
void test_queue_init(void) {
    queue_t_int queue;
    
    // NULL queue parameter
    ASSERT_EQ_INT(queue_init_int(NULL, NULL), ERR_PARAM_INVALID);

    // Valid initialization
    ASSERT_EQ_INT(queue_init_int(&queue, mock_int_free), OK);
    ASSERT_EQ_PTR(queue.head, NULL);
    ASSERT_EQ_PTR(queue.tail, NULL);
    ASSERT_EQ_FNPTR(queue.data_free, mock_int_free);
}

/* Edge Cases: Enqueue Operations */
void test_queue_enqueue(void) {
    queue_t_int queue;
    queue_init_int(&queue, NULL);

    // Invalid parameters
    ASSERT_EQ_INT(queue_enqueue_int(NULL, 10), ERR_PARAM_INVALID);

    // Enqueue into empty queue
    ASSERT_EQ_INT(queue_enqueue_int(&queue, 10), OK);
    ASSERT_EQ_PTR(queue.head, queue.tail);
    ASSERT_EQ_INT(queue.head->data, 10);
    ASSERT_EQ_PTR(queue.head->next, NULL);

    // Enqueue into non-empty queue (Verifying FIFO structure)
    ASSERT_EQ_INT(queue_enqueue_int(&queue, 20), OK);
    ASSERT_EQ_INT(queue.head->data, 10);           // Head remains the same (oldest element)
    ASSERT_EQ_INT(queue.head->next->data, 20);     // Next element is the new one
    ASSERT_EQ_INT(queue.tail->data, 20);           // Tail moves to the newest element

    queue_free_int(&queue);
}

/* Edge Cases: Peek Operations */
void test_queue_peek(void) {
    queue_t_int queue;
    int data = 0;
    
    queue_init_int(&queue, NULL);

    // Invalid parameters
    ASSERT_EQ_INT(queue_peek_int(NULL, &data), ERR_PARAM_INVALID);
    ASSERT_EQ_INT(queue_peek_int(&queue, NULL), ERR_PARAM_INVALID);

    // Peek empty queue
    ASSERT_EQ_INT(queue_peek_int(&queue, &data), ERR_EMPTY);

    // Peek populated queue
    queue_enqueue_int(&queue, 42);
    ASSERT_EQ_INT(queue_peek_int(&queue, &data), OK);
    ASSERT_EQ_INT(data, 42);

    // Ensure peek didn't modify the queue
    ASSERT_EQ_INT(queue.head->data, 42);
    ASSERT_EQ_INT(queue_peek_int(&queue, &data), OK);

    queue_enqueue_int(&queue, 99);
    ASSERT_EQ_INT(queue_peek_int(&queue, &data), OK);
    ASSERT_EQ_INT(data, 42); // Should STILL peek the oldest element in a queue

    queue_free_int(&queue);
}

/* Edge Cases: Dequeue Operations */
void test_queue_dequeue(void) {
    queue_t_int queue;
    int data = 0;
    
    queue_init_int(&queue, NULL);

    // Invalid parameters
    ASSERT_EQ_INT(queue_dequeue_int(NULL, &data), ERR_PARAM_INVALID);
    ASSERT_EQ_INT(queue_dequeue_int(&queue, NULL), ERR_PARAM_INVALID);

    // Dequeue empty queue
    ASSERT_EQ_INT(queue_dequeue_int(&queue, &data), ERR_EMPTY);

    // Populate queue: [Front: 10 -> 20 -> 30 :Back]
    queue_enqueue_int(&queue, 10);
    queue_enqueue_int(&queue, 20);
    queue_enqueue_int(&queue, 30);

    // Multi-element FIFO dequeue validation
    ASSERT_EQ_INT(queue_dequeue_int(&queue, &data), OK);
    ASSERT_EQ_INT(data, 10);
    ASSERT_EQ_INT(queue.head->data, 20);

    ASSERT_EQ_INT(queue_dequeue_int(&queue, &data), OK);
    ASSERT_EQ_INT(data, 20);
    ASSERT_EQ_INT(queue.head->data, 30);
    ASSERT_EQ_PTR(queue.head, queue.tail); // Only one element left

    ASSERT_EQ_INT(queue_dequeue_int(&queue, &data), OK);
    ASSERT_EQ_INT(data, 30);
    ASSERT_EQ_PTR(queue.head, NULL);
    ASSERT_EQ_PTR(queue.tail, NULL);

    // Queue is empty again
    ASSERT_EQ_INT(queue_dequeue_int(&queue, &data), ERR_EMPTY);

    queue_free_int(&queue);
}

/* Edge Cases: Custom data_free Callback and Cleanup */
void test_queue_free_and_callbacks(void) {
    queue_t_int queue;
    int data = 0;
    
    custom_free_calls = 0;
    queue_init_int(&queue, mock_int_free);

    ASSERT_EQ_INT(queue_free_int(NULL), ERR_PARAM_INVALID);

    queue_enqueue_int(&queue, 1);
    queue_enqueue_int(&queue, 2);
    queue_enqueue_int(&queue, 3);

    // Trigger data_free via queue_dequeue (which internally deletes the node)
    queue_dequeue_int(&queue, &data);
    ASSERT_EQ_INT(custom_free_calls, 1);

    // Trigger data_free via queue_free on remaining 2 nodes
    ASSERT_EQ_INT(queue_free_int(&queue), OK);
    ASSERT_EQ_INT(custom_free_calls, 3);
    ASSERT_EQ_PTR(queue.head, NULL);
    ASSERT_EQ_PTR(queue.tail, NULL);

    // Freeing an already freed/empty queue
    ASSERT_EQ_INT(queue_free_int(&queue), OK);
}

int main(void) {
    error_init();

    printf("=== QUEUE INT TEST SUITE ===\n");
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
