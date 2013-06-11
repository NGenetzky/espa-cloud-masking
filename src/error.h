#ifndef ERROR_H
#define ERROR_H

#include "bool.h"

#define ERROR(message, module) \
          Error((message), (module), (__FILE__), (long)(__LINE__), true)

#define RETURN_ERROR(message, module, status) \
          {Error((message), (module), (__FILE__), (long)(__LINE__), false); \
	   return (status);}

void Error(const char *message, const char *module, 
           const char *source, long line, bool done);

#endif
