// Copyright (c) 2019 Daniel Abrecht
// SPDX-License-Identifier: WTFPL OR MIT

#ifndef DPAPARSER_H
#define DPAPARSER_H

#include <stddef.h>

#define GEN_UNPACK(...) __VA_ARGS__

typedef long gen_integer_t;
typedef bool gen_boolean_t;

typedef struct gen_string {
  char* data;
  size_t length;
} gen_string_t;

typedef struct gen_map_entry {
  gen_string_t key;
  gen_string_t value;
} gen_map_entry_t;

typedef struct gen_list {
  gen_string_t* entries;
  size_t length;
} gen_list_t;

typedef struct gen_map_t {
  gen_map_entry_t* entries;
  size_t length;
} gen_map_t;

#endif
