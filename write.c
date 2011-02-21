#ifdef BTYACC_BUILD_USE_CONFIGURATION_HEADER
#include "build.h"   // System settings from the build configuration
#endif

#include <stdlib.h>
#include <stdarg.h>
#include "write.h"

void BtYacc_putc(char c, FILE* f)
{
    if (putc(c, f) == EOF)
    {
       perror("BtYacc_putc");
       abort();
    }
}

void BtYacc_puts(char const * text, FILE* f)
{
    if (fputs(text, f) == EOF)
    {
       perror("BtYacc_puts");
       abort();
    }
}

void BtYacc_printf(FILE* f, char const * format, ...)
{
  va_list vl;

  va_start(vl, format);

  if (vfprintf(f, format, vl) < 0)
  {
     perror("BtYacc: vfprintf");
     va_end(vl);
     abort();
  }

  va_end(vl);
}
