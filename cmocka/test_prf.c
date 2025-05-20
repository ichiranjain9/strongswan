#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <crypto/prfs/prf.h>
#include <library.h>
#include <asn1/oid.h>

static void test_pseudo_random_function_from_oid(void **state)
{
    (void)state;
    assert_int_equal(pseudo_random_function_from_oid(OID_HMAC_SHA1), PRF_HMAC_SHA1);
    assert_int_equal(pseudo_random_function_from_oid(OID_HMAC_SHA256), PRF_HMAC_SHA2_256);
    assert_int_equal(pseudo_random_function_from_oid(OID_HMAC_SHA384), PRF_HMAC_SHA2_384);
    assert_int_equal(pseudo_random_function_from_oid(OID_HMAC_SHA512), PRF_HMAC_SHA2_512);
    assert_int_equal(pseudo_random_function_from_oid(OID_MD5), PRF_UNDEFINED);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_pseudo_random_function_from_oid),
    };
    library_init(NULL, "cmocka");
    int ret = cmocka_run_group_tests(tests, NULL, NULL);
    library_deinit();
    return ret;
}
