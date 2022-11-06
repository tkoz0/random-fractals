#pragma once

#include <stdlib.h>

char *read_text_file(const char *fname);

char *read_stdin_text();

size_t read_binary_file(const char *fname, void **buf);
