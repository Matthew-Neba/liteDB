// unit test most of the server functions
#include "server.h"
#include <assert.h>

// test the server functions
bool test_get_response()
{
    // test string
    ValueType type = STRING;
    void *value = "hello";

    ValueType response_type = 0;
    int len = 0;

    char *response;

    response = get_response(type, value);

    // check first byte
    memcpy(&response_type, response, 1);
    if (type != STRING)
    {
        fprintf(stderr, "response type should be string\n");
        return false;
    }

    memcpy(&len, response + 1, 4);
    if (len != strlen(value))
    {
        fprintf(stderr, "string response length should be %ld\n", strlen(value));
        return false;
    }

    if (strcmp(value, response + 5) != 0)
    {
        fprintf(stderr, "string response value should be %s\n", (char *)value);
        return false;
    }

    // test int
    type = INTEGER;
    int i = 123;
    value = &i;

    response = get_response(type, value);

    // check first byte
    memcpy(&response_type, response, 1);
    if (type != INTEGER)
    {
        fprintf(stderr, "response type should be integer\n");
        return false;
    }

    memcpy(&len, response + 1, 4);
    if (len != 4)
    {
        fprintf(stderr, "integer response length should be 4\n");
        return false;
    }

    if (i != *(int *)(response + 5))
    {
        fprintf(stderr, "integer response value should be %d\n", i);
        return false;
    }

    // test float
    type = FLOAT;
    float f = 123.456;
    value = &f;

    response = get_response(type, value);

    // check first byte
    memcpy(&response_type, response, 1);
    if (type != FLOAT)
    {
        fprintf(stderr, "response type should be float\n");
        return false;
    }

    memcpy(&len, response + 1, 4);
    if (len != 4)
    {
        fprintf(stderr, "float response length should be 4\n");
        return false;
    }

    if (f != *(float *)(response + 5))
    {
        fprintf(stderr, "float response value should be %f\n", f);
        return false;
    }

    // all tests passed
    return true;
}

// ! Don't have to unit test all fucntions, some are covered by integration tests

bool test_null_response()
{
    char *response = null_response();

    SerialType type = 0;

    // check first byte
    memcpy(&type, response, 1);
    if (type != SER_NIL)
    {
        fprintf(stderr, "response type should be nil\n");
        return false;
    }

    if (*(int *)(response + 1) != 0)
    {
        fprintf(stderr, "nil response length should be 0\n");
        return false;
    }

    return true;
}

bool test_error_response()
{
    char *response = error_response("error message");

    SerialType type = 0;

    // check first byte
    memcpy(&type, response, 1);
    if (type != SER_ERR)
    {
        fprintf(stderr, "response type should be error\n");
        return false;
    }

    if (*(int *)(response + 1) != strlen("error message"))
    {
        fprintf(stderr, "error response length should be 13\n");
        return false;
    }

    if (strcmp("error message", response + 5) != 0)
    {
        fprintf(stderr, "error response message should be 'error message'\n");
        return false;
    }

    return true;
}

bool test_avl_iterate_response()
{

    // create AVL tree
    AVLNode *root = avl_init("1", 1);
    avl_insert(root, "2", 2);
    avl_insert(root, "3", 3);

    AVLNode *start = avl_search_float(root, 1);
    if (start == NULL)
    {
        fprintf(stderr, "node with key 1 not found\n");
        return false;
    }

    char *response = avl_iterate_response(root, start, 2);

    // check first byte
    if (response[0] != SER_ARR)
    {
        fprintf(stderr, "response type should be array\n");
        return false;
    }

    // check length
    if (*(int *)(response + 1) != 2 * 2)
    {
        fprintf(stderr, "response length should be 2\n");
        return false;
    }

    // check first element type
    if (response[5] != SER_STR)
    {
        fprintf(stderr, "first element type should be string\n");
        return false;
    }

    return true;
}

bool test_parse_cmd_string()
{
    char *cmd_string = "SET key";
    Command *cmd = parse_cmd_string(cmd_string, strlen(cmd_string));

    if (strcmp(cmd->name, "SET") != 0)
    {
        fprintf(stderr, "command name should be 'SET'\n");
        return false;
    }

    if (cmd->num_args != 1)
    {
        fprintf(stderr, "number of arguments should be 1\n");
        return false;
    }

    if (strcmp(cmd->args[0], "key") != 0)
    {
        fprintf(stderr, "first argument should be 'key'\n");
        return false;
    }

    return true;
}

void test_init()
{
    global_table = hcreate(INIT_TABLE_SIZE);
}

void test_reset()
{
    hfree_table(global_table);
}

bool test_string_commands()
{

    test_init();

    char *cmdString = "SET key value";
    // set this to true, don't want to write to aof file in a tests
    bool aof_restore = true;

    Command *cmd = parse_cmd_string(cmdString, 13);
    set_command(cmd, aof_restore);

    // check if global table has the key
    HashNode *fetched_node = hget(global_table, "key");
    if (!fetched_node)
    {
        fprintf(stderr, "key  not found in global table, was not set\n");
        return false;
    }

    if (strcmp(fetched_node->value, "value") != 0)
    {
        fprintf(stderr, "incorrect value set\n");
        return false;
    }

    // test del command
    cmdString = "DEL key";
    cmd = parse_cmd_string(cmdString, strlen(cmdString));
    del_command(cmd, aof_restore);

    // check if key was deleted
    fetched_node = hget(global_table, "key");
    if (fetched_node)
    {
        fprintf(stderr, "key was not deleted\n");
        return false;
    }

    // reset global table
    test_reset();

    return true;
}

bool test_meta_commands()
{

    test_init();

    // set this to true, don't want to write to aof file in a tests
    bool aof_restore = true;

    // insert a string,hashtable, list, sorted set
    char *cmdString = "SET key value";
    Command *cmd = parse_cmd_string(cmdString, strlen(cmdString));
    set_command(cmd, aof_restore);

    cmdString = "HSET hash key value";
    cmd = parse_cmd_string(cmdString, strlen(cmdString));
    hset_command(cmd, aof_restore);

    cmdString = "LPUSH list value";
    cmd = parse_cmd_string(cmdString, strlen(cmdString));
    lpush_command(cmd, aof_restore);

    cmdString = "ZADD sortedset 1 value";
    cmd = parse_cmd_string(cmdString, strlen(cmdString));
    zadd_command(cmd, aof_restore);

    // test keys command
    char *response = keys_command();
    if (response[0] != SER_ARR)
    {
        fprintf(stderr, "response type should be array\n");
        return false;
    }

    if (*(int *)(response + 1) != 4)
    {
        fprintf(stderr, "response length should be %d\n", 4);
        return false;
    }

    // test del command
    cmdString = "DEL key";

    cmd = parse_cmd_string(cmdString, strlen(cmdString));
    del_command(cmd, aof_restore);

    // check if key was deleted
    HashNode *fetched_node = hget(global_table, "key");
    if (fetched_node)
    {
        fprintf(stderr, "key was not deleted\n");
        return false;
    }

    // test flushall command
    cmdString = "FLUSHALL";
    cmd = parse_cmd_string(cmdString, strlen(cmdString));
    flushall_cmd(cmd, aof_restore);

    // check if all keys were deleted
    int num_keys = global_table->size;

    if (num_keys != 0)
    {
        fprintf(stderr, "all keys were not deleted\n");
        return false;
    }

    // reset global table
    test_reset();

    return true;
}

bool test_hashtable_commands()
{
    // set this to true, don't want to write to aof file in a tests
    bool aof_restore = true;

    test_init();

    // test hset command
    char *cmdString = "HSET hash key value";
    Command *cmd = parse_cmd_string(cmdString, strlen(cmdString));
    hset_command(cmd, aof_restore);

    // check if global table has the hash
    HashNode *fetched_node = hget(global_table, "hash");
    if (!fetched_node)
    {
        fprintf(stderr, "hash not found in global table, was not set\n");
        return false;
    }

    // check if hash has the key
    HashNode *hash_node = hget(fetched_node->value, "key");
    if (!hash_node)
    {
        fprintf(stderr, "key not found in hash, was not set\n");
        return false;
    }

    if (strcmp(hash_node->value, "value") != 0)
    {
        fprintf(stderr, "incorrect value set\n");
        return false;
    }

    // test get command
    cmdString = "HGET hash key";
    cmd = parse_cmd_string(cmdString, strlen(cmdString));
    char *response = hget_command(cmd);

    if (response[0] != SER_STR)
    {
        fprintf(stderr, "response type should be string\n");
        return false;
    }

    if (*(int *)(response + 1) != strlen("value"))
    {
        fprintf(stderr, "response length should be %ld\n", strlen("value"));
        return false;
    }

    if (strcmp("value", response + 5) != 0)
    {
        fprintf(stderr, "response value should be 'value'\n");
        return false;
    }

    // test hdel command
    cmdString = "HDEL hash key";
    cmd = parse_cmd_string(cmdString, strlen(cmdString));
    hdel_command(cmd, aof_restore);

    // check if key was deleted
    hash_node = hget(fetched_node->value, "key");
    if (hash_node)
    {
        fprintf(stderr, "key was not deleted\n");
        return false;
    }

    return true;
}

bool test_list_commands() {
    // set this to true, don't want to write to aof file in a tests
    bool aof_restore = true;

    test_init();

    // test lpush command
    char *cmdString = "LPUSH list value";
    Command *cmd = parse_cmd_string(cmdString, strlen(cmdString));
    lpush_command(cmd, aof_restore);

    // check if global table has the list
    HashNode *fetched_node = hget(global_table, "list");
    if (!fetched_node)
    {
        fprintf(stderr, "list not found in global table, was not set\n");
        return false;
    }

    // check if list has the value
    ListNode *list_node = lget(fetched_node->value, 0);
    if (!list_node)
    {
        fprintf(stderr, "value not found in list, was not set\n");
        return false;
    }

    if (strcmp(list_node->value, "value") != 0)
    {
        fprintf(stderr, "incorrect value set\n");
        return false;
    }

    // test rpush command
    cmdString = "RPUSH list value2";
    cmd = parse_cmd_string(cmdString, strlen(cmdString));
    rpush_command(cmd, aof_restore);

    // check if list has the value
    list_node = lget(fetched_node->value, 1);
    if (!list_node)
    {
        fprintf(stderr, "value not found in list, was not set\n");
        return false;
    }

    if (strcmp(list_node->value, "value2") != 0)
    {
        fprintf(stderr, "incorrect value set\n");
        return false;
    }

    return true;
} 

int main()
{

    assert(test_get_response());
    assert(test_null_response());
    assert(test_error_response());
    assert(test_avl_iterate_response());
    assert(test_parse_cmd_string());
    assert(test_string_commands());
    assert(test_hashtable_commands());






    assert(test_meta_commands());

    printf("All tests passed\n");
    return 0;
}