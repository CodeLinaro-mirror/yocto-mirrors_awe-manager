/* MIT License
**
** Copyright (c) 2024 DSP Concepts, Inc.
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**/


#ifndef IDBG_TYPES_H
#define IDBG_TYPES_H

#include "idbg.h"

typedef int Bool;

// typedef uint8 uint8_t;
typedef unsigned char uint8 ;

#define FALSE 0
#define TRUE  1

#define MAX_IDBG_ARGS      1024  // "one two" is one arg, 'one two' as well
#define MAX_IDBG_DIR_LEVEL 5
#define MAX_IDBG_DIR_PATH  256

struct idbg_st {
  idbgtableentry_t *currentTable_p;
  idbgtableentry_t *TableLevel_p[MAX_IDBG_DIR_LEVEL];
  int               table_level;
  char             *argv_ptrs[MAX_IDBG_ARGS];

  idbgtableentry_t *dyn_tables[100];  // todo: do realloc probably here
  int               nr_dyn_tables;

  char              dir_path[MAX_IDBG_DIR_PATH];
  Bool              argPassError; 
  void             *userdata_p; // application specific data

  idbglib_print_func *print_fct_p;
  void               *print_fd_p;
};


#endif // IDBG_TYPES_H
