#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <credentials/sets/cert_cache.h>
#include <library.h>
#include <credentials/certificates/certificate.h>
#include <utils/chunk.h>
#include <stdlib.h>

typedef struct stub_certificate_t stub_certificate_t;
struct stub_certificate_t {
    certificate_t public;
    certificate_type_t type;
    int ref;
    bool issued_by_result;
    int issued_by_calls;
};

METHOD(certificate_t, stub_get_type, certificate_type_t,
       stub_certificate_t *this)
{
    return this->type;
}

METHOD(certificate_t, stub_equals, bool,
       stub_certificate_t *this, certificate_t *other)
{
    return &this->public == other;
}

METHOD(certificate_t, stub_issued_by, bool,
       stub_certificate_t *this, certificate_t *issuer,
       signature_params_t **scheme)
{
    this->issued_by_calls++;
    if (scheme) {
        *scheme = NULL;
    }
    return this->issued_by_result;
}

METHOD(certificate_t, stub_get_ref, certificate_t*,
       stub_certificate_t *this)
{
    this->ref++;
    return &this->public;
}

METHOD(certificate_t, stub_destroy, void,
       stub_certificate_t *this)
{
    if (--this->ref == 0) {
        free(this);
    }
}

/* unused methods */
METHOD(certificate_t, stub_get_subject, identification_t*, stub_certificate_t *this) { return NULL; }
METHOD(certificate_t, stub_has_subject, id_match_t, stub_certificate_t *this, identification_t *id) { return ID_MATCH_NONE; }
METHOD(certificate_t, stub_get_issuer, identification_t*, stub_certificate_t *this) { return NULL; }
METHOD(certificate_t, stub_has_issuer, id_match_t, stub_certificate_t *this, identification_t *id) { return ID_MATCH_NONE; }
METHOD(certificate_t, stub_get_public_key, public_key_t*, stub_certificate_t *this) { return NULL; }
METHOD(certificate_t, stub_get_validity, bool, stub_certificate_t *this, time_t *when, time_t *nb, time_t *na) { return TRUE; }
METHOD(certificate_t, stub_get_encoding, bool, stub_certificate_t *this, cred_encoding_type_t type, chunk_t *enc) { return FALSE; }

static stub_certificate_t *stub_certificate_create(certificate_type_t type, bool result)
{
    stub_certificate_t *this;
    INIT(this,
        .public = {
            .get_type = _stub_get_type,
            .get_subject = _stub_get_subject,
            .has_subject = _stub_has_subject,
            .get_issuer = _stub_get_issuer,
            .has_issuer = _stub_has_issuer,
            .issued_by = _stub_issued_by,
            .get_public_key = _stub_get_public_key,
            .get_validity = _stub_get_validity,
            .get_encoding = _stub_get_encoding,
            .equals = _stub_equals,
            .get_ref = _stub_get_ref,
            .destroy = _stub_destroy,
        },
        .type = type,
        .ref = 1,
        .issued_by_result = result,
        .issued_by_calls = 0,
    );
    return this;
}

static void test_cert_cache_basic(void **state)
{
    (void)state;
    cert_cache_t *cache = cert_cache_create();
    stub_certificate_t *subject = stub_certificate_create(CERT_X509, true);
    stub_certificate_t *issuer = stub_certificate_create(CERT_X509, true);

    assert_true(cache->issued_by(cache, &subject->public, &issuer->public, NULL));
    assert_int_equal(subject->issued_by_calls, 1);

    assert_true(cache->issued_by(cache, &subject->public, &issuer->public, NULL));
    assert_int_equal(subject->issued_by_calls, 1);

    enumerator_t *e = cache->set.create_cert_enumerator(&cache->set, CERT_ANY, KEY_ANY, NULL, false);
    certificate_t *cert;
    assert_true(e->enumerate(e, &cert));
    assert_ptr_equal(cert, &subject->public);
    assert_false(e->enumerate(e, &cert));
    e->destroy(e);

    cache->flush(cache, CERT_ANY);
    cache->destroy(cache);
    subject->public.destroy(&subject->public);
    issuer->public.destroy(&issuer->public);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_cert_cache_basic),
    };
    library_init(NULL, "cmocka");
    int ret = cmocka_run_group_tests(tests, NULL, NULL);
    library_deinit();
    return ret;
}
