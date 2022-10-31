#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json.h"

// uses asserts for parsing, fails if there is an error

// wrapper for fprintf(stderr,..)
static void _write_error(const char *f, ...)
{
    va_list args;
    va_start(args,f);
    vfprintf(stderr,f,args);
    va_end(args);
}

// convert double to string
// TODO get this to work like python3 float.__repr__
// for now just %.16e (max 23 chars + null), it preserves exact value
// handle NaN, and Infinity separately
static void _double_to_string(double num, char *buf)
{
    if (num != num) // NaN
        sprintf(buf,"NaN");
    else if (num == INFINITY)
        sprintf(buf,"Infinity");
    else if (num == -INFINITY)
        sprintf(buf,"-Infinity");
    else
        sprintf(buf,"%.16le",num);
}

// increment i to next non whitespace
static void _skip_whitespace(const char *data, size_t *i)
{
    for (;;)
    {
        char c = data[*i];
        switch (c)
        {
        case ' ':
        case '\n':
        case '\r':
        case '\t':
            ++(*i);
            break;
        default:
            return;
        }
    }
}

// returns the string length
// dest == NULL for counting length only, otherwise it writes string result
// TODO does not support \u escape
static size_t _process_string(const char *data, size_t *i, char *dest)
{
    size_t ret = 0;
    for (;;)
    {
        char c;
        switch(data[*i])
        {
        case '\\': // escaped character
            ++(*i);
            switch (data[*i])
            {
            case '"':  c = '"';  break;
            case '\\': c = '\\'; break;
            case '/':  c = '/';  break;
            case 'b':  c = '\b'; break;
            case 'f':  c = '\f'; break;
            case 'n':  c = '\n'; break;
            case 'r':  c = '\r'; break;
            case 't':  c = '\t'; break;
            case 'u':
                _write_error("json: \\u string escape not supported\n");
                assert(0);
            default:
                _write_error("json: invalid escape sequence: \\%c\n",data[*i]);
                assert(0);
            }
            break;
        case '"': // end of string
            ++(*i);
            return ret;
        case '\0':
            _write_error("json: found null when parsing string\n");
            assert(0);
        default:
            c = data[*i];
            break;
        }
        if (dest)
            *(dest++) = c;
        ++(*i);
        ++ret;
    }
}

// return newly allocated string, NULL for error
static char *_read_string(const char *data, size_t *i)
{
    assert(data[*i] == '"');
    ++(*i);
    size_t j = *i;
    size_t length = _process_string(data,&j,NULL);
    if (length == -1)
        return NULL;
    char *ret = malloc(length+1);
    assert(ret);
    ret[length] = '\0';
    _process_string(data,i,ret);
    return ret;
}

typedef union { json_int as_int; json_float as_float; } _jnum_t;

// some macros for char type
#define _IS_SIGN(c) ((c) == '-' || (c) == '+')
#define _IS_DIGIT(c) ('0' <= (c) && (c) <= '9')
#define _IS_NONZERO_DIGIT(c) ('1' <= (c) && (c) <= '9')
#define _IS_E(c) ((c) == 'e' || (c) == 'E')

// return 0 for integer, 1 for floating point
// format from json.org -?(0|[1-9]\d*)(.\d+)?([Ee][+\-]?\d+)?
// TODO this does not properly check validity of number formats
static int _read_number(const char *data, size_t *i, _jnum_t *ret)
{
    if (data[*i] == 'N') // NaN
    {
        ++(*i);
        assert(data[*i] == 'a'); ++(*i);
        assert(data[*i] == 'N'); ++(*i);
        ret->as_float = NAN;
        return 1;
    }
    if (data[*i] == 'I' || (data[*i] == '-' && data[*i+1] == 'I')) // Infinity
    {
        bool neg = data[*i] == '-';
        if (data[*i] == '-')
            ++(*i);
        ++(*i);
        assert(data[*i] == 'n'); ++(*i);
        assert(data[*i] == 'f'); ++(*i);
        assert(data[*i] == 'i'); ++(*i);
        assert(data[*i] == 'n'); ++(*i);
        assert(data[*i] == 'i'); ++(*i);
        assert(data[*i] == 't'); ++(*i);
        assert(data[*i] == 'y'); ++(*i);
        ret->as_float = neg ? -INFINITY : INFINITY;
        return 1;
    }
    size_t start = *i;
    assert(data[*i] == '.' || _IS_SIGN(data[*i]) || _IS_DIGIT(data[*i]));
    bool is_float = false;
    if (_IS_SIGN(data[*i]))
        ++(*i);
    while (_IS_DIGIT(data[*i]))
        ++(*i);
    if (data[*i] == '.') // fraction part
    {
        is_float = true;
        ++(*i);
        while (_IS_DIGIT(data[*i]))
            ++(*i);
    }
    if (_IS_E(data[*i])) // exponent part
    {
        is_float = true;
        ++(*i);
        if (_IS_SIGN(data[*i]))
            ++(*i);
        while (_IS_DIGIT(data[*i]))
            ++(*i);
    }
    // i should point to first char after the number
    char *tmp = malloc(*i-start+1);
    assert(tmp);
    tmp[*i-start] = '\0';
    memcpy(tmp,data+start,*i-start);
    if (is_float)
        ret->as_float = atof(tmp);
    else
        ret->as_int = atoll(tmp);
    free(tmp);
    return is_float;
}

// returns a newly allocated copy of the string
static char *_copy_string(const char *s)
{
    char *ret = malloc(strlen(s)+1);
    assert(ret);
    strcpy(ret,s);
    return ret;
}

// string hash algorithm used in java
static size_t _str_hash(const char *s)
{
    size_t h = 0;
    while (*s)
        h = (31*h) + (size_t)(*(s++));
    return h;
}

// resize the hash table, growth factor is about 1.25
static void _object_resize(json_object obj)
{
    struct _json_object_bucket *newbuckets;
    size_t new_alloc = obj->alloc + (obj->alloc >> 2) + 2;
    newbuckets = calloc(obj->alloc,sizeof(*newbuckets));
    assert(newbuckets);
    for (size_t i = 0; i < obj->alloc; ++i) // hash objects into new table
    {
        if (!obj->buckets[i].key) // null = empty
            continue;
        size_t h = _str_hash(obj->buckets[i].key);
        h %= new_alloc;
        while (newbuckets[h].key) // find empty bucket
        {
            ++h;
            if (h == new_alloc)
                h = 0;
        }
        newbuckets[h].key = obj->buckets[i].key;
        newbuckets[h].value = obj->buckets[i].value;
    }
    if (obj->buckets) // only allocated if not null
        free(obj->buckets);
    // assign new hash table
    obj->buckets = newbuckets;
    obj->alloc = new_alloc;
}

// hash item into table, first resize if ~75% load factor is reached
// key string is copied for insertion if needed
void json_object_insert(json_object obj, const char *key, json_value value)
{
    if (obj->alloc == 0 || (obj->len >= obj->alloc - (obj->alloc >> 2) - 1))
        _object_resize(obj);
    size_t h = _str_hash(key);
    h %= obj->alloc;
    while (obj->buckets[h].key)
    {
        if (!strcmp(obj->buckets[h].key,key)) // found key, replace
        {
            json_destroy(obj->buckets[h].value);
            obj->buckets[h].value = value;
            ++obj->len;
            return;
        }
        ++h;
        if (h == obj->alloc)
            h = 0;
    }
    // insert in empty bucket found
    obj->buckets[h].key = _copy_string(key);
    obj->buckets[h].value = value;
    ++obj->len;
}

static json_value _read_value(const char *data, size_t *i);

static json_object _read_object(const char *data, size_t *i)
{
    assert(data[*i] == '{');
    ++(*i);
    _skip_whitespace(data,i);
    json_object obj;
    obj->buckets = NULL;
    obj->len = obj->alloc = 0;
    for (;;)
    {
        if (data[*i] == '}')
            break;
        _skip_whitespace(data,i);
        char *s = _read_string(data,i);
        assert(s);
        _skip_whitespace(data,i);
        assert(data[*i] == ':');
        ++(*i);
        json_value value = _read_value(data,i);
        assert(value);
        json_object_insert(obj,s,value);
        free(s);
        if (data[*i] == ',')
            ++(*i);
        else
        {
            assert(data[*i] == '}');
            break;
        }
    }
    ++(*i);
    return obj;
}

// resize array with growth factor about 1.25
static void _array_resize(json_array arr)
{
    json_value *newarray;
    size_t new_alloc = arr->alloc + (arr->alloc >> 2) + 1;
    newarray = realloc(arr->array,new_alloc);
    assert(newarray);
    arr->array = newarray;
    arr->alloc = new_alloc;
}

// insert into array, resizing if necessary
void json_array_append(json_array arr, json_value value)
{
    if (arr->len == arr->alloc)
        _array_resize(arr);
    arr->array[arr->len++] = value;
}

static json_array _read_array(const char *data, size_t *i)
{
    assert(data[*i] == '[');
    ++(*i);
    _skip_whitespace(data,i);
    json_array arr;
    arr->array = NULL;
    arr->len = arr->alloc = 0;
    for (;;)
    {
        if (data[*i] == ']')
            break;
        json_value value = _read_value(data,i);
        assert(value);
        json_array_append(arr,value);
        if (data[*i] == ',')
            ++(*i);
        else
        {
            assert(data[*i] == ']');
            break;
        }
    }
    ++(*i);
    return arr;
}

static json_value _read_value(const char *data, size_t *i)
{
    _skip_whitespace(data,i);
    json_value ret = malloc(sizeof(json_value));
    assert(ret);
    if (data[*i] == '"')
    {
        ret->type = JSON_STRING;
        ret->value.as_str = _read_string(data,i);
    }
    else if (data[*i] == '.' || _IS_SIGN(data[*i]) || _IS_DIGIT(data[*i])
        || data[*i] == 'N' || data[*i] == 'I')
    {
        _jnum_t val;
        int r = _read_number(data,i,&val);
        if (r == 0)
        {
            ret->type = JSON_NUMBER_INT;
            ret->value.as_int = val.as_int;
        }
        else if (r == 1)
        {
            ret->type = JSON_NUMBER_FLOAT;
            ret->value.as_float = val.as_float;
        }
        else assert(0);
    }
    else if (data[*i] == '{')
    {
        ret->type = JSON_OBJECT;
        ret->value.as_object = _read_object(data,i);
    }
    else if (data[*i] == '[')
    {
        ret->type = JSON_ARRAY;
        ret->value.as_array = _read_array(data,i);
    }
    else if (data[*i] == 'f')
    {
        ++(*i);
        assert(data[*i] == 'a'); ++(*i);
        assert(data[*i] == 'l'); ++(*i);
        assert(data[*i] == 's'); ++(*i);
        assert(data[*i] == 'e'); ++(*i);
        ret->type = JSON_BOOL;
        ret->value.as_bool = false;
    }
    else if (data[*i] == 't')
    {
        ++(*i);
        assert(data[*i] == 'r'); ++(*i);
        assert(data[*i] == 'u'); ++(*i);
        assert(data[*i] == 'e'); ++(*i);
        ret->type = JSON_BOOL;
        ret->value.as_bool = true;
    }
    else if (data[*i] == 'n')
    {
        ++(*i);
        assert(data[*i] == 'u'); ++(*i);
        assert(data[*i] == 'l'); ++(*i);
        assert(data[*i] == 'l'); ++(*i);
        ret->type = JSON_NULL;
    }
    _skip_whitespace(data,i);
    return ret;
}

// creates a JSON object (newly allocated) from a string
json_value json_load(const char *data)
{
    size_t i = 0;
    json_value ret = _read_value(data,&i);
    assert(!data[i]);
    return ret;
}

static void _dump_chars(char *buf, size_t *i, char ch, size_t count)
{
    while (count--)
    {
        if (buf)
            buf[*i] = ch;
        ++(*i);
    }
}

static void _dump_spaces(char *buf, size_t *i, size_t count)
{
    _dump_chars(buf,i,' ',count);
}

static void _dump_string(char *buf, size_t *i, const char *s)
{
    while (*s)
    {
        if (buf)
            buf[*i] = *s;
        ++(*i);
        ++s;
    }
}

static void _json_dump_str(char *buf, size_t *i, char *value)
{
     _dump_string(buf,i,"\"");
    char *ptr = value;
    while (*ptr)
    {
        switch (*ptr)
        {
        case '"':  _dump_string(buf,i,"\\\""); break;
        case '\\': _dump_string(buf,i,"\\\\"); break;
        case '\b': _dump_string(buf,i,"\\b");  break;
        case '\f': _dump_string(buf,i,"\\f");  break;
        case '\n': _dump_string(buf,i,"\\n");  break;
        case '\r': _dump_string(buf,i,"\\r");  break;
        case '\t': _dump_string(buf,i,"\\t");  break;
        default: _dump_chars(buf,i,*ptr,1); break;
        }
        ++ptr;
    }
    _dump_string(buf,i,"\"");
}

// writes it to buf if buf is non null
static void _json_dump_helper(json_value data, uint32_t depth,
                        uint32_t indent, char *buf, size_t *i)
{
    switch (data->type)
    {
    case JSON_STRING:
        _json_dump_str(buf,i,data->value.as_str);
        break;
    case JSON_NUMBER_INT:
        char ibuf[20];
        int ibuflen = sprintf(ibuf,"%ld",data->value.as_int);
        if (buf)
            memcpy(buf+*i,ibuf,ibuflen);
        *i += ibuflen;
        break;
    case JSON_NUMBER_FLOAT:
        char fbuf[24];
        _double_to_string(data->value.as_float,fbuf);
        if (buf)
            memcpy(buf+*i,fbuf,strlen(fbuf));
        *i += strlen(fbuf);
        break;
    case JSON_OBJECT:
        if (!data->value.as_object)
        {
            _dump_string(buf,i,"{}");
            break;
        }
        json_object optr = data->value.as_object;
        _dump_string(buf,i,"{");
        if (indent)
            _dump_string(buf,i,"\n");
        _dump_spaces(buf,i,depth+indent);
        _json_dump_str(buf,i,optr->key);
        _dump_string(buf,i,":");
        if (indent)
            _dump_spaces(buf,i,1);
        _json_dump_helper(optr->value,depth+indent,indent,buf,i);
        while (optr->next)
        {
            optr = optr->next;
            _dump_string(buf,i,",");
            if (indent)
                _dump_string(buf,i,"\n");
            _dump_spaces(buf,i,depth+indent);
            _json_dump_str(buf,i,optr->key);
            _dump_string(buf,i,":");
            if (indent)
                _dump_spaces(buf,i,1);
            _json_dump_helper(optr->value,depth+indent,indent,buf,i);
        }
        if (indent)
            _dump_string(buf,i,"\n");
        _dump_spaces(buf,i,depth);
        _dump_string(buf,i,"}");
        break;
    case JSON_ARRAY:
        if (!data->value.as_array)
        {
            _dump_string(buf,i,"[]");
            break;
        }
        json_array *aptr = data->value.as_array;
        _dump_string(buf,i,"[");
        if (indent)
            _dump_string(buf,i,"\n");
        _dump_spaces(buf,i,depth+indent);
        _json_dump_helper(aptr->value,depth+indent,indent,buf,i);
        while (aptr->next)
        {
            aptr = aptr->next;
            _dump_string(buf,i,",");
            if (indent)
                _dump_string(buf,i,"\n");
            _dump_spaces(buf,i,depth+indent);
            _json_dump_helper(aptr->value,depth+indent,indent,buf,i);
        }
        if (indent)
            _dump_string(buf,i,"\n");
        _dump_spaces(buf,i,depth);
        _dump_string(buf,i,"]");
        break;
    case JSON_BOOL:
        const char *v = data->value.as_bool ? "true" : "false";
        if (buf)
            memcpy(buf+*i,v,strlen(v));
        *i += strlen(v);
        break;
    case JSON_NULL:
        if (buf)
            memcpy(buf+*i,"null",4);
        *i += 4;
        break;
    default:
        assert(0);
    }
}

// if indent is 0, outputs as a compact string (newly allocated)
char *json_dump(json_value *data, uint32_t indent)
{
    size_t len = 0;
    _json_dump_helper(data,0,indent,NULL,&len);
    char *ret = malloc(len+1);
    assert(ret);
    ret[len] = '\0';
    size_t i = 0;
    _json_dump_helper(data,0,indent,ret,&i);
    return ret;
}

// free all the dynamically allocated memory associated with a JSON value
void json_destroy(json_value *json)
{
    if (json->type == JSON_STRING)
        free(json->value.as_str);
    else if (json->type == JSON_OBJECT)
    {
        json_object *ptr = json->value.as_object;
        while (ptr)
        {
            json_object *ptr_old = ptr;
            free(ptr->key);
            json_destroy(ptr->value);
            ptr = ptr->next;
            free(ptr_old);
        }
    }
    else if (json->type == JSON_ARRAY)
    {
        json_array *ptr = json->value.as_array;
        while (ptr)
        {
            json_array *ptr_old = ptr;
            json_destroy(ptr->value);
            ptr = ptr->next;
            free(ptr_old);
        }
    }
    free(json);
}

size_t json_array_len(json_array *array)
{
    size_t ret = 0;
    while (array)
    {
        array = array->next;
        ++ret;
    }
    return ret;
}

size_t json_object_len(json_object *object)
{
    size_t ret = 0;
    while (object)
    {
        object = object->next;
        ++ret;
    }
    return ret;
}

// index an array, return NULL if out of bounds
json_value json_array_get(json_array *array, size_t index)
{
    if (!array)
        return NULL;
    while (index--)
    {
        if (!array->next)
            return NULL;
        array = array->next;
    }
    return array->value;
}

// get the value corresponding to a key, return NULL if not exist
json_value json_object_get(json_object *object, const char *key)
{
    if (!object)
        return NULL;
    while (object)
    {
        if (!strcmp(key,object->key))
            return object->value;
        object = object->next;
    }
    return NULL;
}

// some code that was used for testing
#if 0
char *read_file(const char *fname)
{
    FILE *f = fopen(fname,"r");
    fseek(f,0,SEEK_END);
    size_t length = ftell(f);
    fseek(f,0,SEEK_SET);
    char *buf = malloc(length+1);
    assert(buf);
    buf[length] = '\0';
    size_t read_length = fread(buf,1,length,f);
    assert(read_length == length);
    fclose(f);
    return buf;
}
#include <assert.h>
int main(int argc, char **argv)
{
    char *data = read_file("a.json");
    json_value *v = json_load(data);
    free(data);
    data = json_dump(v,0);
    printf("%s\n",data);
    free(data);
    //printf("type = %u\n",v->type);
    json_destroy(v);
    //_jnum_t num;
    //int ret = _read_number(argv[1],&i,&num);
    //if (ret == 0)
    //    printf("%lu (int)\n",num.as_int);
    //else if (ret == 1)
    //    printf("%lf (float)\n",num.as_float);
    //else
    //    printf("error\n");
    //char *s = _read_string(argv[1],&i);
    //assert(s);
    //printf("%s\n",s);
    //free(s);
    return 0;
}
#endif
