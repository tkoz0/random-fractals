#include "utils.h"
#include <assert.h>
#include <stdio.h>

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
