#include "utils.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// read entire file contents into newly allocated null terminated string
char *read_text_file(const char *fname)
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

// read stdin until end and return a newly allocated null terminated string
char *read_stdin_text()
{
    char buf[512];
    char *result = malloc(1);
    assert(result);
    result[0] = '\0';
    size_t size = 1;
    size_t ret;
    // read buffer and append each time
    while ((ret = fread(buf,1,512,stdin)))
    {
        size_t newsize = size + ret;
        result = realloc(result,newsize);
        assert(result);
        memcpy(result+size-1,buf,ret);
        result[newsize-1] = '\0';
        size = newsize;
    }
    return result;
}

// sets buf to a newly allocated array of the file contents
// returns number of bytes read
size_t read_binary_file(const char *fname, void **buf)
{
    FILE *f = fopen(fname,"rb");
    fseek(f,0,SEEK_END);
    size_t length = ftell(f);
    fseek(f,0,SEEK_SET);
    *buf = malloc(length);
    assert(*buf);
    size_t read_length = fread(*buf,1,length,f);
    assert(read_length == length);
    fclose(f);
    return read_length;
}
