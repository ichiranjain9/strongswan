#include "test_suite.h"

#include <selectors/sec_label.h>
#include <utils/chunk.h>

static const char label_str[] = "system_u:object_r:ipsec_spd_t:s0";

START_TEST(test_sec_label_from_string)
{
    sec_label_t *label;
    chunk_t enc = chunk_create((u_char*)label_str, strlen(label_str) + 1);

    label = sec_label_from_string(label_str);
    ck_assert(label);
    ck_assert(chunk_equals(label->get_encoding(label), enc));
    ck_assert_str_eq(label->get_string(label), label_str);
    label->destroy(label);
}
END_TEST

START_TEST(test_sec_label_from_encoding_no_null)
{
    sec_label_t *label;
    chunk_t enc = chunk_from_str(label_str);
    chunk_t expected = chunk_create((u_char*)label_str, strlen(label_str) + 1);

    label = sec_label_from_encoding(enc);
    ck_assert(label);
    ck_assert(chunk_equals(label->get_encoding(label), expected));
    ck_assert_str_eq(label->get_string(label), label_str);
    label->destroy(label);
}
END_TEST

START_TEST(test_sec_label_from_encoding_empty)
{
    ck_assert(!sec_label_from_encoding(chunk_empty));
}
END_TEST

START_TEST(test_sec_label_mode_from_string)
{
    sec_label_mode_t mode;

    ck_assert(sec_label_mode_from_string("simple", &mode));
    ck_assert_int_eq(mode, SEC_LABEL_MODE_SIMPLE);
    ck_assert(sec_label_mode_from_string("system", &mode));
    ck_assert_int_eq(mode, SEC_LABEL_MODE_SYSTEM);
    ck_assert(!sec_label_mode_from_string("invalid", &mode));
}
END_TEST

START_TEST(test_sec_label_mode_default)
{
    ck_assert_int_eq(sec_label_mode_default(), SEC_LABEL_MODE_SIMPLE);
}
END_TEST

Suite *sec_label_suite_create()
{
    Suite *s;
    TCase *tc;

    s = suite_create("sec_label");

    tc = tcase_create("from_string");
    tcase_add_test(tc, test_sec_label_from_string);
    suite_add_tcase(s, tc);

    tc = tcase_create("from_encoding");
    tcase_add_test(tc, test_sec_label_from_encoding_no_null);
    tcase_add_test(tc, test_sec_label_from_encoding_empty);
    suite_add_tcase(s, tc);

    tc = tcase_create("modes");
    tcase_add_test(tc, test_sec_label_mode_from_string);
    tcase_add_test(tc, test_sec_label_mode_default);
    suite_add_tcase(s, tc);

    return s;
}
