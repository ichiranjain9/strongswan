#include "test_suite.h"

#include <database/database_factory.h>

typedef struct {
    database_t public;
    bool destroyed;
} stub_db_t;

static db_driver_t db_get_driver(stub_db_t *this) { return DB_SQLITE; }
static void db_destroy(stub_db_t *this) { this->destroyed = TRUE; free(this); }

static database_t* stub_db_create(char *uri)
{
    if (streq(uri, "stub://"))
    {
        stub_db_t *this;
        INIT(this,
            .public = {
                .get_driver = (void*)db_get_driver,
                .destroy = (void*)db_destroy,
            },
            .destroyed = FALSE,
        );
        return &this->public;
    }
    return NULL;
}

START_TEST(test_database_factory_create)
{
    database_factory_t *factory = database_factory_create();
    database_t *db;

    factory->add_database(factory, stub_db_create);
    db = factory->create(factory, "stub://");
    ck_assert(db);
    ck_assert_int_eq(db->get_driver(db), DB_SQLITE);
    db->destroy(db);
    factory->destroy(factory);
}
END_TEST

START_TEST(test_database_factory_remove)
{
    database_factory_t *factory = database_factory_create();
    database_t *db;

    factory->add_database(factory, stub_db_create);
    factory->remove_database(factory, stub_db_create);
    db = factory->create(factory, "stub://");
    ck_assert(!db);
    factory->destroy(factory);
}
END_TEST

Suite *database_factory_suite_create()
{
    Suite *s;
    TCase *tc;

    s = suite_create("database_factory");

    tc = tcase_create("create");
    tcase_add_test(tc, test_database_factory_create);
    suite_add_tcase(s, tc);

    tc = tcase_create("remove");
    tcase_add_test(tc, test_database_factory_remove);
    suite_add_tcase(s, tc);

    return s;
}
