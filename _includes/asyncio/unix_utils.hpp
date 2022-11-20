#pragma once

#include <unistd.h>

[[noreturn]] void throw_errno(const char* what);
void set_non_blocking(int fd, bool enabled);
