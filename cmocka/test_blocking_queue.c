#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <library.h>
#include <collections/blocking_queue.h>
#include <stdlib.h>

static void test_basic_enqueue_dequeue(void **state)
{
    (void)state;
    blocking_queue_t *q = blocking_queue_create();
    int a = 1, b = 2, c = 3;
    q->enqueue(q, &a);
    q->enqueue(q, &b);
    q->enqueue(q, &c);
    assert_ptr_equal(q->dequeue(q), &a);
    assert_ptr_equal(q->dequeue(q), &b);
    assert_ptr_equal(q->dequeue(q), &c);
    q->destroy(q);
}

static void test_destroy_function(void **state)
{
    (void)state;
    blocking_queue_t *q = blocking_queue_create();
    int *a = malloc(sizeof(int));
    int *b = malloc(sizeof(int));
    q->enqueue(q, a);
    q->enqueue(q, b);
    q->destroy_function(q, free);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_basic_enqueue_dequeue),
        cmocka_unit_test(test_destroy_function),
    };
    library_init(NULL, "cmocka");
    int ret = cmocka_run_group_tests(tests, NULL, NULL);
    library_deinit();
    return ret;
}
