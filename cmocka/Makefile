CC=gcc
CFLAGS=-I../src/libstrongswan -I../src/libstrongswan/tests $(shell pkg-config --cflags cmocka) -include ../config.h
LDFLAGS=-L../src/libstrongswan/.libs -lstrongswan -lpthread $(shell pkg-config --libs cmocka)

TESTS=test_blocking_queue test_callback_cred test_cert_cache test_pkcs5 test_prf

all: $(TESTS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TESTS)
