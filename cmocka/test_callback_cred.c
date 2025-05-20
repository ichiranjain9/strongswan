#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <credentials/sets/callback_cred.h>
#include <library.h>
#include <credentials/keys/shared_key.h>
#include <utils/chunk.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    const char **strings;
    size_t count;
    size_t index;
} cb_data_t;

static shared_key_t* shared_cb(void *data, shared_key_type_t type,
                               identification_t *me, identification_t *other,
                               id_match_t *match_me, id_match_t *match_other)
{
    cb_data_t *d = data;
    (void)me; (void)other; (void)match_me; (void)match_other;
    if (d->index >= d->count) {
        return NULL;
    }
    char *s = strdup(d->strings[d->index++]);
    return shared_key_create(type, chunk_from_str(s));
}

static void test_shared_enumerator(void **state)
{
    (void)state;
    const char *vals[] = {"one", "two"};
    cb_data_t data = { vals, 2, 0 };
    callback_cred_t *cred = callback_cred_create_shared(shared_cb, &data);
    enumerator_t *e = cred->set.create_shared_enumerator(&cred->set, SHARED_IKE,
                                                         NULL, NULL);
    shared_key_t *key;
    assert_true(e->enumerate(e, &key, NULL, NULL));
    assert_int_equal(key->get_type(key), SHARED_IKE);
    assert_memory_equal(key->get_key(key).ptr, "one", 3);
    key->destroy(key);

    assert_true(e->enumerate(e, &key, NULL, NULL));
    assert_memory_equal(key->get_key(key).ptr, "two", 3);
    key->destroy(key);

    assert_false(e->enumerate(e, &key, NULL, NULL));
    e->destroy(e);
    cred->destroy(cred);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_shared_enumerator),
    };
    library_init(NULL, "cmocka");
    int ret = cmocka_run_group_tests(tests, NULL, NULL);
    library_deinit();
    return ret;
}
