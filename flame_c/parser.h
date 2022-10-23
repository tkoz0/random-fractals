/*
Parse JSON data into data structures for flame rendering
*/

#pragma once

#include "json.h"
#include "types.h"

typedef struct flame_list flame_list;

struct flame_list
{
    flame_t value;
    flame_list *next;
};

flame_list *flames_from_json(json_value *data);

void destroy_flame_list(flame_list *f);
