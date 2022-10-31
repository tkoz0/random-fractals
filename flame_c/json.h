/*
JSON
The number type is split into integer and floating point
Arrays are dynamically resized like C++ vector
Objects are represented as hash tables
*/

#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef int64_t json_int;
typedef double json_float;

typedef struct json_value* json_value;
typedef struct json_object* json_object;
typedef struct json_array* json_array;

typedef enum
{
    JSON_STRING,
    JSON_NUMBER_INT,
    JSON_NUMBER_FLOAT,
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_BOOL,
    JSON_NULL
}
json_type;

// json object, determine type using the type information
struct json_value
{ 
    json_type type;
    union
    {
        char *as_str;
        json_int as_int;
        json_float as_float;
        json_object as_object;
        json_array as_array;
        bool as_bool;
    }
    value;
};

struct _json_object_bucket
{
    char *key;
    json_value value;
};

// key,value object map as a linked list of (k,v) pairs
struct json_object
{
    struct _json_object_bucket *buckets;
    size_t len, alloc;
};

struct json_array
{
    json_value *array;
    size_t len, alloc;
};

json_value json_load(const char *data);

char *json_dump(json_value data, uint32_t indent);

void json_destroy(json_value json);

size_t json_array_len(json_array array);

size_t json_object_len(json_object object);

json_value json_array_get(json_array array, size_t index);

json_value json_object_get(json_object object, const char *key);

// TODO array/object init functions

void json_object_insert(json_object obj, const char *key, json_value value);

void json_array_append(json_array arr, json_value value);
