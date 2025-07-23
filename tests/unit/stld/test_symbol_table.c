/**
 * @file test_symbol_table.c
 * @brief Unit tests for STLD symbol table management
 * @version 1.0.0
 * @date 2025-07-23
 * 
 * Comprehensive unit tests for symbol table operations including symbol
 * insertion, lookup, resolution, and duplicate handling. Tests cover
 * global and local symbols, weak symbols, and symbol visibility.
 */

#include "unity.h"
#include "symbol_table.h"
#include "error.h"
#include <string.h>

/* Test fixture data */
static symbol_table_t* test_table;
static error_context_t* test_error_ctx;

/**
 * @brief Setup function called before each test
 */
void setUp(void) {
    test_error_ctx = error_context_create();
    TEST_ASSERT_NOT_NULL(test_error_ctx);
    
    test_table = symbol_table_create(test_error_ctx);
    TEST_ASSERT_NOT_NULL(test_table);
}

/**
 * @brief Teardown function called after each test
 */
void tearDown(void) {
    if (test_table != NULL) {
        symbol_table_destroy(test_table);
        test_table = NULL;
    }
    
    if (test_error_ctx != NULL) {
        error_context_destroy(test_error_ctx);
        test_error_ctx = NULL;
    }
}

/**
 * @brief Test symbol table creation and destruction
 */
void test_symbol_table_lifecycle(void) {
    symbol_table_t* table = symbol_table_create(test_error_ctx);
    TEST_ASSERT_NOT_NULL(table);
    
    /* Initial state */
    TEST_ASSERT_EQUAL_size_t(0, symbol_table_size(table));
    TEST_ASSERT_TRUE(symbol_table_is_empty(table));
    
    symbol_table_destroy(table);
    
    /* Test creation with NULL error context */
    table = symbol_table_create(NULL);
    TEST_ASSERT_NULL(table);
    
    /* Test destruction with NULL */
    symbol_table_destroy(NULL); /* Should not crash */
}

/**
 * @brief Test basic symbol insertion
 */
void test_symbol_insertion_basic(void) {
    /* Create test symbols */
    symbol_t symbol1 = {
        .name = "main",
        .type = SYMBOL_TYPE_FUNCTION,
        .binding = SYMBOL_BINDING_GLOBAL,
        .visibility = SYMBOL_VISIBILITY_DEFAULT,
        .section_index = 1,
        .value = 0x1000,
        .size = 0x100
    };
    
    symbol_t symbol2 = {
        .name = "global_var",
        .type = SYMBOL_TYPE_OBJECT,
        .binding = SYMBOL_BINDING_GLOBAL,
        .visibility = SYMBOL_VISIBILITY_DEFAULT,
        .section_index = 2,
        .value = 0x2000,
        .size = 0x20
    };
    
    /* Insert symbols */
    symbol_handle_t handle1 = symbol_table_insert(test_table, &symbol1);
    symbol_handle_t handle2 = symbol_table_insert(test_table, &symbol2);
    
    TEST_ASSERT_NOT_EQUAL(SYMBOL_HANDLE_INVALID, handle1);
    TEST_ASSERT_NOT_EQUAL(SYMBOL_HANDLE_INVALID, handle2);
    TEST_ASSERT_NOT_EQUAL(handle1, handle2);
    
    /* Verify table state */
    TEST_ASSERT_EQUAL_size_t(2, symbol_table_size(test_table));
    TEST_ASSERT_FALSE(symbol_table_is_empty(test_table));
    
    /* Verify no errors occurred */
    TEST_ASSERT_FALSE(error_context_has_error(test_error_ctx));
}

/**
 * @brief Test symbol lookup by name
 */
void test_symbol_lookup_by_name(void) {
    /* Insert test symbols */
    symbol_t symbol1 = { .name = "func1", .type = SYMBOL_TYPE_FUNCTION, .binding = SYMBOL_BINDING_GLOBAL, .value = 0x1000 };
    symbol_t symbol2 = { .name = "func2", .type = SYMBOL_TYPE_FUNCTION, .binding = SYMBOL_BINDING_LOCAL, .value = 0x2000 };
    symbol_t symbol3 = { .name = "var1", .type = SYMBOL_TYPE_OBJECT, .binding = SYMBOL_BINDING_GLOBAL, .value = 0x3000 };
    
    symbol_handle_t h1 = symbol_table_insert(test_table, &symbol1);
    symbol_handle_t h2 = symbol_table_insert(test_table, &symbol2);
    symbol_handle_t h3 = symbol_table_insert(test_table, &symbol3);
    
    TEST_ASSERT_NOT_EQUAL(SYMBOL_HANDLE_INVALID, h1);
    TEST_ASSERT_NOT_EQUAL(SYMBOL_HANDLE_INVALID, h2);
    TEST_ASSERT_NOT_EQUAL(SYMBOL_HANDLE_INVALID, h3);
    
    /* Test successful lookups */
    symbol_handle_t found1 = symbol_table_lookup(test_table, "func1");
    symbol_handle_t found2 = symbol_table_lookup(test_table, "func2");
    symbol_handle_t found3 = symbol_table_lookup(test_table, "var1");
    
    TEST_ASSERT_EQUAL(h1, found1);
    TEST_ASSERT_EQUAL(h2, found2);
    TEST_ASSERT_EQUAL(h3, found3);
    
    /* Test failed lookup */
    symbol_handle_t not_found = symbol_table_lookup(test_table, "nonexistent");
    TEST_ASSERT_EQUAL(SYMBOL_HANDLE_INVALID, not_found);
    
    /* Test NULL name */
    symbol_handle_t null_lookup = symbol_table_lookup(test_table, NULL);
    TEST_ASSERT_EQUAL(SYMBOL_HANDLE_INVALID, null_lookup);
}

/**
 * @brief Test symbol retrieval by handle
 */
void test_symbol_retrieval(void) {
    /* Insert test symbol */
    symbol_t original = {
        .name = "test_symbol",
        .type = SYMBOL_TYPE_FUNCTION,
        .binding = SYMBOL_BINDING_GLOBAL,
        .visibility = SYMBOL_VISIBILITY_DEFAULT,
        .section_index = 1,
        .value = 0x1234,
        .size = 0x56
    };
    
    symbol_handle_t handle = symbol_table_insert(test_table, &original);
    TEST_ASSERT_NOT_EQUAL(SYMBOL_HANDLE_INVALID, handle);
    
    /* Retrieve symbol */
    const symbol_t* retrieved = symbol_table_get(test_table, handle);
    TEST_ASSERT_NOT_NULL(retrieved);
    
    /* Verify symbol data */
    TEST_ASSERT_EQUAL_STRING("test_symbol", retrieved->name);
    TEST_ASSERT_EQUAL_INT(SYMBOL_TYPE_FUNCTION, retrieved->type);
    TEST_ASSERT_EQUAL_INT(SYMBOL_BINDING_GLOBAL, retrieved->binding);
    TEST_ASSERT_EQUAL_INT(SYMBOL_VISIBILITY_DEFAULT, retrieved->visibility);
    TEST_ASSERT_EQUAL_INT(1, retrieved->section_index);
    TEST_ASSERT_EQUAL_HEX32(0x1234, retrieved->value);
    TEST_ASSERT_EQUAL_size_t(0x56, retrieved->size);
    
    /* Test invalid handle */
    const symbol_t* invalid = symbol_table_get(test_table, SYMBOL_HANDLE_INVALID);
    TEST_ASSERT_NULL(invalid);
    
    /* Test out-of-bounds handle */
    symbol_handle_t bad_handle = (symbol_handle_t)(symbol_table_size(test_table) + 100);
    const symbol_t* bad_retrieval = symbol_table_get(test_table, bad_handle);
    TEST_ASSERT_NULL(bad_retrieval);
}

/**
 * @brief Test duplicate symbol handling
 */
void test_duplicate_symbol_handling(void) {
    /* Insert first symbol */
    symbol_t symbol1 = {
        .name = "duplicate_name",
        .type = SYMBOL_TYPE_FUNCTION,
        .binding = SYMBOL_BINDING_GLOBAL,
        .value = 0x1000
    };
    
    symbol_handle_t handle1 = symbol_table_insert(test_table, &symbol1);
    TEST_ASSERT_NOT_EQUAL(SYMBOL_HANDLE_INVALID, handle1);
    
    /* Try to insert symbol with same name */
    symbol_t symbol2 = {
        .name = "duplicate_name",
        .type = SYMBOL_TYPE_OBJECT,
        .binding = SYMBOL_BINDING_GLOBAL,
        .value = 0x2000
    };
    
    symbol_handle_t handle2 = symbol_table_insert(test_table, &symbol2);
    
    /* Should fail for global symbols with same name */
    TEST_ASSERT_EQUAL(SYMBOL_HANDLE_INVALID, handle2);
    TEST_ASSERT_TRUE(error_context_has_error(test_error_ctx));
    
    /* Clear error for next test */
    error_context_clear(test_error_ctx);
    
    /* Local symbols with same name should be allowed in different scopes */
    symbol_t local_symbol = {
        .name = "duplicate_name",
        .type = SYMBOL_TYPE_OBJECT,
        .binding = SYMBOL_BINDING_LOCAL,
        .value = 0x3000
    };
    
    symbol_handle_t local_handle = symbol_table_insert(test_table, &local_symbol);
    TEST_ASSERT_NOT_EQUAL(SYMBOL_HANDLE_INVALID, local_handle);
    TEST_ASSERT_FALSE(error_context_has_error(test_error_ctx));
}

/**
 * @brief Test weak symbol handling
 */
void test_weak_symbol_handling(void) {
    /* Insert weak symbol */
    symbol_t weak_symbol = {
        .name = "weak_function",
        .type = SYMBOL_TYPE_FUNCTION,
        .binding = SYMBOL_BINDING_WEAK,
        .value = 0x1000,
        .size = 0x100
    };
    
    symbol_handle_t weak_handle = symbol_table_insert(test_table, &weak_symbol);
    TEST_ASSERT_NOT_EQUAL(SYMBOL_HANDLE_INVALID, weak_handle);
    
    /* Insert strong symbol with same name (should override weak) */
    symbol_t strong_symbol = {
        .name = "weak_function",
        .type = SYMBOL_TYPE_FUNCTION,
        .binding = SYMBOL_BINDING_GLOBAL,
        .value = 0x2000,
        .size = 0x200
    };
    
    symbol_handle_t strong_handle = symbol_table_insert(test_table, &strong_symbol);
    TEST_ASSERT_NOT_EQUAL(SYMBOL_HANDLE_INVALID, strong_handle);
    
    /* Lookup should return strong symbol */
    symbol_handle_t found = symbol_table_lookup(test_table, "weak_function");
    const symbol_t* symbol = symbol_table_get(test_table, found);
    
    TEST_ASSERT_NOT_NULL(symbol);
    TEST_ASSERT_EQUAL_INT(SYMBOL_BINDING_GLOBAL, symbol->binding);
    TEST_ASSERT_EQUAL_HEX32(0x2000, symbol->value);
    TEST_ASSERT_EQUAL_size_t(0x200, symbol->size);
}

/**
 * @brief Test symbol iteration
 */
void test_symbol_iteration(void) {
    /* Insert multiple symbols */
    const char* names[] = {"sym1", "sym2", "sym3", "sym4"};
    const size_t count = sizeof(names) / sizeof(names[0]);
    
    for (size_t i = 0; i < count; i++) {
        symbol_t symbol = {
            .name = names[i],
            .type = SYMBOL_TYPE_FUNCTION,
            .binding = SYMBOL_BINDING_GLOBAL,
            .value = (uint32_t)(0x1000 + i * 0x100)
        };
        
        symbol_handle_t handle = symbol_table_insert(test_table, &symbol);
        TEST_ASSERT_NOT_EQUAL(SYMBOL_HANDLE_INVALID, handle);
    }
    
    /* Iterate through symbols */
    size_t iteration_count = 0;
    bool found_symbols[4] = {false, false, false, false};
    
    symbol_iterator_t iter = symbol_table_begin(test_table);
    while (symbol_iterator_is_valid(&iter)) {
        const symbol_t* symbol = symbol_iterator_get(&iter);
        TEST_ASSERT_NOT_NULL(symbol);
        
        /* Find which symbol this is */
        for (size_t i = 0; i < count; i++) {
            if (strcmp(symbol->name, names[i]) == 0) {
                found_symbols[i] = true;
                TEST_ASSERT_EQUAL_HEX32(0x1000 + i * 0x100, symbol->value);
                break;
            }
        }
        
        iteration_count++;
        symbol_iterator_next(&iter);
    }
    
    /* Verify all symbols were found */
    TEST_ASSERT_EQUAL_size_t(count, iteration_count);
    for (size_t i = 0; i < count; i++) {
        TEST_ASSERT_TRUE_MESSAGE(found_symbols[i], "Symbol not found during iteration");
    }
}

/**
 * @brief Test symbol filtering by type
 */
void test_symbol_filtering(void) {
    /* Insert symbols of different types */
    symbol_t function = { .name = "func", .type = SYMBOL_TYPE_FUNCTION, .binding = SYMBOL_BINDING_GLOBAL };
    symbol_t object = { .name = "obj", .type = SYMBOL_TYPE_OBJECT, .binding = SYMBOL_BINDING_GLOBAL };
    symbol_t section = { .name = "sect", .type = SYMBOL_TYPE_SECTION, .binding = SYMBOL_BINDING_LOCAL };
    
    symbol_table_insert(test_table, &function);
    symbol_table_insert(test_table, &object);
    symbol_table_insert(test_table, &section);
    
    /* Filter by type */
    symbol_list_t* functions = symbol_table_filter_by_type(test_table, SYMBOL_TYPE_FUNCTION);
    symbol_list_t* objects = symbol_table_filter_by_type(test_table, SYMBOL_TYPE_OBJECT);
    symbol_list_t* sections = symbol_table_filter_by_type(test_table, SYMBOL_TYPE_SECTION);
    
    TEST_ASSERT_NOT_NULL(functions);
    TEST_ASSERT_NOT_NULL(objects);
    TEST_ASSERT_NOT_NULL(sections);
    
    TEST_ASSERT_EQUAL_size_t(1, symbol_list_size(functions));
    TEST_ASSERT_EQUAL_size_t(1, symbol_list_size(objects));
    TEST_ASSERT_EQUAL_size_t(1, symbol_list_size(sections));
    
    /* Verify correct symbols were filtered */
    const symbol_t* func_sym = symbol_list_get(functions, 0);
    const symbol_t* obj_sym = symbol_list_get(objects, 0);
    const symbol_t* sect_sym = symbol_list_get(sections, 0);
    
    TEST_ASSERT_EQUAL_STRING("func", func_sym->name);
    TEST_ASSERT_EQUAL_STRING("obj", obj_sym->name);
    TEST_ASSERT_EQUAL_STRING("sect", sect_sym->name);
    
    /* Cleanup */
    symbol_list_destroy(functions);
    symbol_list_destroy(objects);
    symbol_list_destroy(sections);
}

/**
 * @brief Test symbol resolution across multiple tables
 */
void test_symbol_resolution(void) {
    /* Create second symbol table */
    symbol_table_t* table2 = symbol_table_create(test_error_ctx);
    TEST_ASSERT_NOT_NULL(table2);
    
    /* Insert symbols in different tables */
    symbol_t symbol1 = { .name = "external_func", .type = SYMBOL_TYPE_FUNCTION, .binding = SYMBOL_BINDING_GLOBAL, .value = 0x1000 };
    symbol_t symbol2 = { .name = "local_func", .type = SYMBOL_TYPE_FUNCTION, .binding = SYMBOL_BINDING_LOCAL, .value = 0x2000 };
    
    symbol_table_insert(test_table, &symbol1);
    symbol_table_insert(table2, &symbol2);
    
    /* Create resolver with both tables */
    symbol_resolver_t* resolver = symbol_resolver_create(test_error_ctx);
    TEST_ASSERT_NOT_NULL(resolver);
    
    symbol_resolver_add_table(resolver, test_table);
    symbol_resolver_add_table(resolver, table2);
    
    /* Test resolution */
    const symbol_t* resolved1 = symbol_resolver_resolve(resolver, "external_func");
    const symbol_t* resolved2 = symbol_resolver_resolve(resolver, "local_func");
    const symbol_t* unresolved = symbol_resolver_resolve(resolver, "missing_func");
    
    TEST_ASSERT_NOT_NULL(resolved1);
    TEST_ASSERT_NOT_NULL(resolved2);
    TEST_ASSERT_NULL(unresolved);
    
    TEST_ASSERT_EQUAL_STRING("external_func", resolved1->name);
    TEST_ASSERT_EQUAL_STRING("local_func", resolved2->name);
    TEST_ASSERT_EQUAL_HEX32(0x1000, resolved1->value);
    TEST_ASSERT_EQUAL_HEX32(0x2000, resolved2->value);
    
    /* Cleanup */
    symbol_resolver_destroy(resolver);
    symbol_table_destroy(table2);
}

/**
 * @brief Test symbol table serialization
 */
void test_symbol_table_serialization(void) {
    /* Insert test symbols */
    symbol_t symbols[] = {
        { .name = "main", .type = SYMBOL_TYPE_FUNCTION, .binding = SYMBOL_BINDING_GLOBAL, .value = 0x1000, .size = 0x100 },
        { .name = "data", .type = SYMBOL_TYPE_OBJECT, .binding = SYMBOL_BINDING_GLOBAL, .value = 0x2000, .size = 0x50 },
        { .name = "local", .type = SYMBOL_TYPE_FUNCTION, .binding = SYMBOL_BINDING_LOCAL, .value = 0x1100, .size = 0x80 }
    };
    
    for (size_t i = 0; i < sizeof(symbols) / sizeof(symbols[0]); i++) {
        symbol_table_insert(test_table, &symbols[i]);
    }
    
    /* Serialize to buffer */
    size_t buffer_size = symbol_table_get_serialized_size(test_table);
    TEST_ASSERT_GREATER_THAN(0, buffer_size);
    
    uint8_t* buffer = malloc(buffer_size);
    TEST_ASSERT_NOT_NULL(buffer);
    
    bool serialize_result = symbol_table_serialize(test_table, buffer, buffer_size);
    TEST_ASSERT_TRUE(serialize_result);
    
    /* Deserialize to new table */
    symbol_table_t* new_table = symbol_table_deserialize(buffer, buffer_size, test_error_ctx);
    TEST_ASSERT_NOT_NULL(new_table);
    
    /* Verify deserialized table */
    TEST_ASSERT_EQUAL_size_t(symbol_table_size(test_table), symbol_table_size(new_table));
    
    for (size_t i = 0; i < sizeof(symbols) / sizeof(symbols[0]); i++) {
        symbol_handle_t handle = symbol_table_lookup(new_table, symbols[i].name);
        TEST_ASSERT_NOT_EQUAL(SYMBOL_HANDLE_INVALID, handle);
        
        const symbol_t* symbol = symbol_table_get(new_table, handle);
        TEST_ASSERT_NOT_NULL(symbol);
        TEST_ASSERT_EQUAL_STRING(symbols[i].name, symbol->name);
        TEST_ASSERT_EQUAL_INT(symbols[i].type, symbol->type);
        TEST_ASSERT_EQUAL_HEX32(symbols[i].value, symbol->value);
    }
    
    /* Cleanup */
    free(buffer);
    symbol_table_destroy(new_table);
}

/**
 * @brief Test symbol table with NULL parameters
 */
void test_symbol_table_null_parameters(void) {
    /* Test operations with NULL table */
    TEST_ASSERT_EQUAL_size_t(0, symbol_table_size(NULL));
    TEST_ASSERT_TRUE(symbol_table_is_empty(NULL));
    TEST_ASSERT_EQUAL(SYMBOL_HANDLE_INVALID, symbol_table_lookup(NULL, "test"));
    TEST_ASSERT_NULL(symbol_table_get(NULL, 0));
    
    /* Test with NULL symbol */
    symbol_handle_t handle = symbol_table_insert(test_table, NULL);
    TEST_ASSERT_EQUAL(SYMBOL_HANDLE_INVALID, handle);
    TEST_ASSERT_TRUE(error_context_has_error(test_error_ctx));
}

/**
 * @brief Main test runner function
 */
int main(void) {
    UNITY_BEGIN();
    
    /* Basic functionality tests */
    RUN_TEST(test_symbol_table_lifecycle);
    RUN_TEST(test_symbol_insertion_basic);
    RUN_TEST(test_symbol_lookup_by_name);
    RUN_TEST(test_symbol_retrieval);
    
    /* Advanced functionality tests */
    RUN_TEST(test_duplicate_symbol_handling);
    RUN_TEST(test_weak_symbol_handling);
    RUN_TEST(test_symbol_iteration);
    RUN_TEST(test_symbol_filtering);
    
    /* Integration tests */
    RUN_TEST(test_symbol_resolution);
    RUN_TEST(test_symbol_table_serialization);
    
    /* Edge case tests */
    RUN_TEST(test_symbol_table_null_parameters);
    
    return UNITY_END();
}
