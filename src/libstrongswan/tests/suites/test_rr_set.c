#include "test_suite.h"

#include <resolver/rr_set.h>
#include <resolver/rr.h>
#include <collections/linked_list.h>
#include <utils/chunk.h>

typedef struct {
    rr_t public;
} stub_rr_t;

static char* rr_get_name(stub_rr_t *this) { return "name"; }
static rr_type_t rr_get_type(stub_rr_t *this) { return RR_TYPE_A; }
static rr_class_t rr_get_class(stub_rr_t *this) { return RR_CLASS_IN; }
static uint32_t rr_get_ttl(stub_rr_t *this) { return 0; }
static chunk_t rr_get_rdata(stub_rr_t *this) { return chunk_empty; }
static void rr_destroy(stub_rr_t *this) { free(this); }

static rr_t* stub_rr_create()
{
    stub_rr_t *this;
    INIT(this,
        .public = {
            .get_name = (void*)rr_get_name,
            .get_type = (void*)rr_get_type,
            .get_class = (void*)rr_get_class,
            .get_ttl = (void*)rr_get_ttl,
            .get_rdata = (void*)rr_get_rdata,
            .destroy = (void*)rr_destroy,
        },
    );
    return &this->public;
}

START_TEST(test_rr_set_basic)
{
    linked_list_t *rrs = linked_list_create();
    linked_list_t *sigs = linked_list_create();
    rr_set_t *set;
    enumerator_t *enumerator;
    rr_t *rr;
    int count = 0;

    rrs->insert_last(rrs, stub_rr_create());
    rrs->insert_last(rrs, stub_rr_create());
    sigs->insert_last(sigs, stub_rr_create());

    set = rr_set_create(rrs, sigs);
    ck_assert(set);

    enumerator = set->create_rr_enumerator(set);
    while (enumerator->enumerate(enumerator, &rr))
    {
        ck_assert(rr);
        count++;
    }
    ck_assert_int_eq(count, 2);
    enumerator->destroy(enumerator);

    enumerator = set->create_rrsig_enumerator(set);
    ck_assert(enumerator);
    count = 0;
    while (enumerator->enumerate(enumerator, &rr))
    {
        count++;
    }
    ck_assert_int_eq(count, 1);
    enumerator->destroy(enumerator);

    set->destroy(set);
}
END_TEST

START_TEST(test_rr_set_no_rrsig)
{
    linked_list_t *rrs = linked_list_create();
    rr_set_t *set;
    enumerator_t *enumerator;
    rr_t *rr;
    int count = 0;

    rrs->insert_last(rrs, stub_rr_create());

    set = rr_set_create(rrs, NULL);
    ck_assert(set);

    enumerator = set->create_rrsig_enumerator(set);
    ck_assert(!enumerator);

    enumerator = set->create_rr_enumerator(set);
    while (enumerator->enumerate(enumerator, &rr))
    {
        count++;
    }
    ck_assert_int_eq(count, 1);
    enumerator->destroy(enumerator);

    set->destroy(set);
}
END_TEST

Suite *rr_set_suite_create()
{
    Suite *s;
    TCase *tc;

    s = suite_create("rr_set");

    tc = tcase_create("basic");
    tcase_add_test(tc, test_rr_set_basic);
    suite_add_tcase(s, tc);

    tc = tcase_create("no_rrsig");
    tcase_add_test(tc, test_rr_set_no_rrsig);
    suite_add_tcase(s, tc);

    return s;
}
