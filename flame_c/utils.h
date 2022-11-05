#pragma once

#include <stdlib.h>

char *read_text_file(const char *fname);

size_t read_binary_file(const char *fname, void **buf);
