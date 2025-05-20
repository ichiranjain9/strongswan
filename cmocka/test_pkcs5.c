#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <crypto/pkcs5.h>
#include <asn1/asn1.h>
#include <library.h>
#include <asn1/oid.h>

static void test_pkcs5_create_destroy(void **state)
{
    chunk_t alg = asn1_algorithmIdentifier(OID_MD5);
    pkcs5_t *pkcs5 = pkcs5_from_algorithmIdentifier(alg, 0);
    assert_null(pkcs5);
    chunk_free(&alg);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_pkcs5_create_destroy),
    };
    library_init(NULL, "cmocka");
    int ret = cmocka_run_group_tests(tests, NULL, NULL);
    library_deinit();
    return ret;
}
