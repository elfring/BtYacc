#ifndef BTYACC_WRITE_H_8A43C6B0A15B404B9F4A8737BEB417DA
#define BTYACC_WRITE_H_8A43C6B0A15B404B9F4A8737BEB417DA

#include <stdio.h>

void BtYacc_putc(char c, FILE* f);
void BtYacc_puts(char const * text, FILE* f);
void BtYacc_printf(FILE* f, char const * format, ...);

#endif
