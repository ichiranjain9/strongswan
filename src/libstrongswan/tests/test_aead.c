#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include <crypto/aead.h>
#include <utils/chunk.h>

/* Stub crypter implementation */
typedef struct {
    crypter_t public;
    uint8_t key[4];
} stub_crypter_t;

static bool stub_encrypt(crypter_t *this_, chunk_t data, chunk_t iv, chunk_t *encrypted)
{
    if (encrypted) {
        *encrypted = chunk_clone(data);
    }
    return TRUE;
}

static bool stub_decrypt(crypter_t *this_, chunk_t data, chunk_t iv, chunk_t *decrypted)
{
    if (decrypted) {
        *decrypted = chunk_clone(data);
    }
    return TRUE;
}

static size_t stub_c_block(crypter_t *this_) { return 1; }
static size_t stub_c_iv(crypter_t *this_) { return 4; }
static size_t stub_c_key(crypter_t *this_) { return 4; }
static bool stub_c_set_key(crypter_t *this_, chunk_t key)
{
    memcpy(((stub_crypter_t*)this_)->key, key.ptr, 4);
    return TRUE;
}
static void stub_c_destroy(crypter_t *this_) { free(this_); }

static stub_crypter_t *stub_crypter_create()
{
    stub_crypter_t *c = malloc(sizeof(*c));
    c->public.encrypt = stub_encrypt;
    c->public.decrypt = stub_decrypt;
    c->public.get_block_size = stub_c_block;
    c->public.get_iv_size = stub_c_iv;
    c->public.get_key_size = stub_c_key;
    c->public.set_key = stub_c_set_key;
    c->public.destroy = stub_c_destroy;
    return c;
}

/* Stub signer implementation */
typedef struct {
    signer_t public;
    uint8_t key[4];
} stub_signer_t;

static uint8_t stub_sum(chunk_t data)
{
    uint8_t s = 0;
    size_t i;
    for (i = 0; i < data.len; i++)
        s += data.ptr[i];
    return s;
}

static bool stub_s_get_signature(signer_t *this_, chunk_t data, uint8_t *buffer)
{
    if (buffer) {
        *buffer = stub_sum(data);
    }
    return TRUE;
}

static bool stub_s_allocate_signature(signer_t *this_, chunk_t data, chunk_t *sig)
{
    sig->len = 1;
    sig->ptr = malloc(1);
    sig->ptr[0] = stub_sum(data);
    return TRUE;
}

static bool stub_s_verify_signature(signer_t *this_, chunk_t data, chunk_t sig)
{
    return sig.len == 1 && sig.ptr[0] == stub_sum(data);
}

static size_t stub_s_block(signer_t *this_) { return 1; }
static size_t stub_s_key(signer_t *this_) { return 4; }
static bool stub_s_set_key(signer_t *this_, chunk_t key)
{
    memcpy(((stub_signer_t*)this_)->key, key.ptr, 4);
    return TRUE;
}
static void stub_s_destroy(signer_t *this_) { free(this_); }

static stub_signer_t *stub_signer_create()
{
    stub_signer_t *s = malloc(sizeof(*s));
    s->public.get_signature = stub_s_get_signature;
    s->public.allocate_signature = stub_s_allocate_signature;
    s->public.verify_signature = stub_s_verify_signature;
    s->public.get_block_size = stub_s_block;
    s->public.get_key_size = stub_s_key;
    s->public.set_key = stub_s_set_key;
    s->public.destroy = stub_s_destroy;
    return s;
}

/* Stub IV generator */
typedef struct {
    iv_gen_t public;
    uint8_t value;
} stub_iv_gen_t;

static bool stub_iv_get(iv_gen_t *this_, uint64_t seq, size_t size, uint8_t *buffer)
{
    memset(buffer, ((stub_iv_gen_t*)this_)->value, size);
    return TRUE;
}
static bool stub_iv_allocate(iv_gen_t *this_, uint64_t seq, size_t size, chunk_t *chunk)
{
    chunk->len = size;
    chunk->ptr = malloc(size);
    memset(chunk->ptr, ((stub_iv_gen_t*)this_)->value, size);
    return TRUE;
}
static void stub_iv_destroy(iv_gen_t *this_) { free(this_); }

static stub_iv_gen_t *stub_iv_gen_create(uint8_t value)
{
    stub_iv_gen_t *g = malloc(sizeof(*g));
    g->public.get_iv = stub_iv_get;
    g->public.allocate_iv = stub_iv_allocate;
    g->public.destroy = stub_iv_destroy;
    g->value = value;
    return g;
}

/* Test fixtures */
static int setup_aead(void **state)
{
    stub_crypter_t *c = stub_crypter_create();
    stub_signer_t *s = stub_signer_create();
    stub_iv_gen_t *g = stub_iv_gen_create(0xAA);
    aead_t *aead = aead_create(&c->public, &s->public, &g->public);
    chunk_t key = chunk_from_chars(1,2,3,4,5,6,7,8);
    assert_true(aead->set_key(aead, key));
    *state = aead;
    return 0;
}

static int teardown_aead(void **state)
{
    aead_t *aead = *state;
    aead->destroy(aead);
    return 0;
}

static void test_encrypt_decrypt_success(void **state)
{
    aead_t *aead = *state;
    chunk_t iv = chunk_from_chars(0xAA,0xAA,0xAA,0xAA);
    chunk_t plain = chunk_from_chars(1,2,3,4);
    chunk_t assoc = chunk_from_chars(9,8);
    chunk_t encrypted;

    assert_true(aead->encrypt(aead, plain, assoc, iv, &encrypted));
    assert_int_equal(encrypted.len, iv.len + plain.len + 1);

    chunk_t iv2 = chunk_create(encrypted.ptr, iv.len);
    chunk_t cipher_sig = chunk_create(encrypted.ptr + iv.len, encrypted.len - iv.len);
    chunk_t out;
    assert_true(aead->decrypt(aead, cipher_sig, assoc, iv2, &out));
    assert_int_equal(out.len, plain.len);
    assert_memory_equal(out.ptr, plain.ptr, plain.len);
    free(encrypted.ptr);
    free(out.ptr);
}

static void test_decrypt_fail_bad_signature(void **state)
{
    aead_t *aead = *state;
    chunk_t iv = chunk_from_chars(0xAA,0xAA,0xAA,0xAA);
    chunk_t plain = chunk_from_chars(1,2,3,4);
    chunk_t assoc = chunk_from_chars(9,8);
    chunk_t encrypted;

    assert_true(aead->encrypt(aead, plain, assoc, iv, &encrypted));
    encrypted.ptr[encrypted.len - 1] ^= 0xFF;

    chunk_t iv2 = chunk_create(encrypted.ptr, iv.len);
    chunk_t cipher_sig = chunk_create(encrypted.ptr + iv.len, encrypted.len - iv.len);
    chunk_t out;
    assert_false(aead->decrypt(aead, cipher_sig, assoc, iv2, &out));
    free(encrypted.ptr);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_encrypt_decrypt_success, setup_aead, teardown_aead),
        cmocka_unit_test_setup_teardown(test_decrypt_fail_bad_signature, setup_aead, teardown_aead),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}

