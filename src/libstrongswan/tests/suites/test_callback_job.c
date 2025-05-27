#include "test_suite.h"

#include <processing/jobs/callback_job.h>

static job_requeue_t run_cb(void *data)
{
    int *cnt = data;
    (*cnt)++;
    return JOB_REQUEUE_NONE;
}

static void cleanup_cb(void *data)
{
    int *cnt = data;
    *cnt = 0xdead;
}

static bool cancel_cb(void *data)
{
    int *cnt = data;
    (*cnt)++;
    return TRUE;
}

START_TEST(test_callback_job_execute)
{
    int cnt = 0;
    callback_job_t *job = callback_job_create(run_cb, &cnt, cleanup_cb, NULL);

    ck_assert(job);
    ck_assert_int_eq(job->job.execute(&job->job).type, JOB_REQUEUE_TYPE_NONE);
    ck_assert_int_eq(cnt, 1);

    job->job.destroy(&job->job);
    ck_assert_int_eq(cnt, 0xdead);
}
END_TEST

START_TEST(test_callback_job_cancel)
{
    int cnt = 0;
    callback_job_t *job = callback_job_create(run_cb, &cnt, cleanup_cb, cancel_cb);

    ck_assert(job);
    ck_assert(job->job.cancel(&job->job));
    ck_assert_int_eq(cnt, 1);

    job->job.destroy(&job->job);
    ck_assert_int_eq(cnt, 0xdead);
}
END_TEST

Suite *callback_job_suite_create()
{
    Suite *s;
    TCase *tc;

    s = suite_create("callback_job");

    tc = tcase_create("execute");
    tcase_add_test(tc, test_callback_job_execute);
    suite_add_tcase(s, tc);

    tc = tcase_create("cancel");
    tcase_add_test(tc, test_callback_job_cancel);
    suite_add_tcase(s, tc);

    return s;
}
