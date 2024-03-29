/*
JSON
The number type is split into integer and floating point
The object and array types are represented as linked lists
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

// key,value object map as a linked list of (k,v) pairs
struct json_object
{
    char *key;
    json_value value;
    json_object next;
};

// value array as a linked list
struct json_array
{
    json_value value;
    json_array next;
};

json_value json_load(const char *data);

char *json_dump(json_value data, uint32_t indent);

void json_destroy(json_value json);

size_t json_array_len(json_array array);

size_t json_object_len(json_object object);

json_value json_array_get(json_array array, size_t index);

json_value json_object_get(json_object object, const char *key);
