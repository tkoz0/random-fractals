#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "types.h"
#include "variations.h"

// wrapper for fprintf(stderr,..)
static void _write_error(const char *f, ...)
{
    va_list args;
    va_start(args,f);
    vfprintf(stderr,f,args);
    va_end(args);
}

// returns a newly allocated copy of the string
static char *_copy_string(const char *s)
{
    char *ret = malloc(strlen(s)+1);
    assert(ret);
    strcpy(ret,s);
    return ret;
}

// expect integer to get value from, using a default if key does not exist
static void _set_u64_from_key(json_object j, const char *k, uint64_t *dest,
                                uint64_t def)
{
    json_value tmp = json_object_get(j,k);
    if (!tmp)
        *dest = def;
    else
    {
        assert(tmp->type == JSON_NUMBER_INT);
        *dest = (uint64_t) tmp->value.as_int;
    }
}

// expect num_t to get value from, using default if key does not exist
static void _set_num_from_key(json_object j, const char *k, num_t *dest,
                                num_t def)
{
    json_value tmp = json_object_get(j,k);
    if (!tmp)
        *dest = def;
    else
    {
        assert(tmp->type == JSON_NUMBER_FLOAT);
        *dest = (num_t) tmp->value.as_float;
    }
}

// expect num_t at array index, fail if array is not long enough
static void _set_num_from_index(json_array j, size_t i, num_t *dest)
{
    json_value tmp = json_array_get(j,i);
    assert(tmp);
    assert(tmp->type == JSON_NUMBER_FLOAT);
    *dest = (num_t) tmp->value.as_float;
}

// defaults and some sanity checks
#define SIZE_X_DEFAULT 512
#define SIZE_Y_DEFAULT 512
#define SIZE_X_MAX 100000
#define SIZE_Y_MAX 100000
#define X_DIM 1.0
#define Y_DIM 1.0
#define DIM_MAX 1e5
#define DENSITY 100

void flame_from_json(json_object jflame, flame_t *flame)
{
    json_value tmp;
    tmp = json_object_get(jflame,"name");
    assert(tmp && tmp->type == JSON_STRING);
    _write_error("parsing flame \"%s\"\n",tmp->value.as_str);
    flame->name = _copy_string(tmp->value.as_str);
    _set_u64_from_key(jflame,"size_x",&flame->size_x,SIZE_X_DEFAULT);
    assert(0 < flame->size_x && flame->size_x < SIZE_X_MAX);
    _set_u64_from_key(jflame,"size_y",&flame->size_y,SIZE_Y_DEFAULT);
    assert(0 < flame->size_y && flame->size_y < SIZE_Y_MAX);
    _set_u64_from_key(jflame,"samples",&flame->samples,
        (uint64_t)flame->size_x*(uint64_t)flame->size_y*DENSITY);
    _set_num_from_key(jflame,"xmin",&flame->xmin,-X_DIM);
    assert(-DIM_MAX < flame->xmin && flame->xmin < DIM_MAX);
    _set_num_from_key(jflame,"xmax",&flame->xmax,X_DIM);
    assert(-DIM_MAX < flame->xmax && flame->xmax < DIM_MAX);
    assert(flame->xmin < flame->xmax);
    _set_num_from_key(jflame,"ymin",&flame->ymin,-Y_DIM);
    assert(-DIM_MAX < flame->ymin && flame->ymin < DIM_MAX);
    _set_num_from_key(jflame,"ymax",&flame->ymax,Y_DIM);
    assert(-DIM_MAX < flame->ymax && flame->ymax < DIM_MAX);
    assert(flame->ymin < flame->ymax);
    json_value jxfvv = json_object_get(jflame,"xforms");
    assert(jxfvv->type == JSON_ARRAY);
    json_array jxfv = jxfvv->value.as_array;
    assert(jxfv);
    flame->xforms_len = json_array_len(jxfv);
    assert(flame->xforms_len);
    _write_error("  has %u xforms\n",flame->xforms_len);
    flame->xforms = malloc(sizeof(xform_t)*flame->xforms_len);
    // xforms loop
    for (size_t i = 0; i < flame->xforms_len; ++i)
    {
        //_write_error("  parsing xform %u\n",i);
        json_value jxfe = json_array_get(jxfv,i);
        assert(jxfe);
        assert(jxfe->type == JSON_OBJECT);
        json_object jxf = jxfe->value.as_object;
        xform_t *xf = flame->xforms+i;
        _set_num_from_key(jxf,"weight",&xf->weight,1.0);
        json_value jvarsv = json_object_get(jxf,"variations");
        assert(jvarsv);
        assert(jvarsv->type == JSON_ARRAY);
        json_array jvars = jvarsv->value.as_array;
        xf->var_len = json_array_len(jvars);
        assert(xf->var_len);
        //_write_error("    has %u vars\n",xf->var_len);
        xf->vars = malloc(sizeof(xf->vars[0])*xf->var_len);
        xf->varw = malloc(sizeof(xf->varw[0])*xf->var_len);
        uint32_t pc_flags = 0;
        // variations loop
        for (size_t j = 0; j < xf->var_len; ++j, jvars = jvars->next)
        {
            //_write_error("      var %u\n",j);
            assert(jvars->value->type == JSON_OBJECT);
            json_object jvar = jvars->value->value.as_object;
            _set_num_from_key(jvar,"weight",xf->varw+j,1.0);
            // TODO support variation number as well as name
            // TODO parse variation parameters
            json_value jnamev = json_object_get(jvar,"name");
            assert(jnamev);
            assert(jnamev->type == JSON_STRING);
            char *varname = jnamev->value.as_str;
            //_write_error("      name %s\n",varname);
            xf->vars[j] = NULL; // find the variation function
            for (size_t k = 0; VARIATIONS[k].name; ++k)
                if (!strcmp(VARIATIONS[k].name,varname))
                {
                    xf->vars[j] = VARIATIONS[k].func;
                    pc_flags |= VARIATIONS[k].flags;
                    break;
                }
            assert(xf->vars[j]);
        }
        xf->pc_flags = pc_flags;
        json_value jafv = json_object_get(jxf,"pre_affine");
        assert(jafv);
        assert(jafv->type == JSON_ARRAY);
        json_array jaf = jafv->value.as_array;
        _set_num_from_index(jaf,0,&xf->pre_affine.a);
        _set_num_from_index(jaf,1,&xf->pre_affine.b);
        _set_num_from_index(jaf,2,&xf->pre_affine.c);
        _set_num_from_index(jaf,3,&xf->pre_affine.d);
        _set_num_from_index(jaf,4,&xf->pre_affine.e);
        _set_num_from_index(jaf,5,&xf->pre_affine.f);
        jafv = json_object_get(jxf,"post_affine");
        assert(jafv);
        assert(jafv->type == JSON_ARRAY);
        jaf = jafv->value.as_array;
        _set_num_from_index(jaf,0,&xf->post_affine.a);
        _set_num_from_index(jaf,1,&xf->post_affine.b);
        _set_num_from_index(jaf,2,&xf->post_affine.c);
        _set_num_from_index(jaf,3,&xf->post_affine.d);
        _set_num_from_index(jaf,4,&xf->post_affine.e);
        _set_num_from_index(jaf,5,&xf->post_affine.f);
    }
}

flame_list flames_from_json(json_value data)
{
    flame_list head = NULL;
    flame_list tail = NULL;
    assert(data->type == JSON_ARRAY);
    json_array jptr = data->value.as_array;
    size_t num_flames = 0;
    while (jptr) // flame loop
    {
        ++num_flames;
        if (!head)
        {
            head = malloc(sizeof(*head));
            assert(head);
            tail = head;
            tail->next = NULL;
        }
        else
        {
            tail->next = malloc(sizeof(*head));
            assert(tail->next);
            tail = tail->next;
            tail->next = NULL;
        }
        assert(jptr->value->type == JSON_OBJECT);
        flame_from_json(jptr->value->value.as_object,&tail->value);
        jptr = jptr->next;
    }
    _write_error("parsed %lu flames\n",num_flames);
    return head;
}

void destroy_flame_list(flame_list f)
{
    while (f)
    {
        flame_list f2 = f;
        f = f->next;
        free(f2->value.name);
        for (size_t i = 0; i < f2->value.xforms_len; ++i)
        {
            xform_t xf = f2->value.xforms[i];
            free(xf.vars);
            free(xf.varw);
        }
        free(f2->value.xforms);
        free(f2);
    }
}
