/*
Parse JSON data into data structures for flame rendering
*/

#pragma once

#include "json.h"
#include "types.h"

typedef struct flame_list* flame_list;

// for a linked list of flames to render
struct flame_list
{
    flame_t value;
    flame_list next;
};

// returns a flame list of entirely newly allocated memory
flame_list flames_from_json(json_value data);

// deallocate all memory pointed to by the flame list structures
void destroy_flame_list(flame_list f);
