#ifdef BTYACC_BUILD_USE_CONFIGURATION_HEADER
#include "build.h"   // System settings from the build configuration
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "log.h"

void BtYacc_logs(char const * text)
{
    if (fputs(text, stderr) == EOF)
    {
       perror("BtYacc: fputs");
       abort();
    }
}

void BtYacc_logf(char const * format, ...)
{
  va_list vl;

  va_start(vl, format);

  if (vfprintf(stderr, format, vl) < 0)
  {
     perror("BtYacc: vfprintf");
     va_end(vl);
     abort();
  }

  va_end(vl);
}
