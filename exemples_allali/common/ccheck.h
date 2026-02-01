#pragma once

#include <stdlib.h>
#include <stdio.h>

#define CHECK(fmt,rfmt,res,fun,...) if (res!=fun(__VA_ARGS__)) \
    { \
      fprintf(stderr,"check failed: " #fun "(" fmt ") returns " rfmt " , expect " rfmt "\n",\
	      __VA_ARGS__,fun(__VA_ARGS__),res); \
      exit(1); \
    }
