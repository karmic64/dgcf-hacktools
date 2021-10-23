#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>


#define ZBUF_SIZE 0x80000


#define isslash(c) ((c)=='/' || (c)=='\\')
char *getfilename(char *fullname)
{
  char *result = fullname;
  char *p = fullname;
  
  while (1)
  {
    char c = *p;
    char n = *(p+1);
    if (!c || !n) break;
    
    if (isslash(c) && !isslash(n))
    {
      result = p+1;
    }
    p++;
  }
  
  /* remove trailing slashes */
  if (isslash(*p))
  {
    while (isslash(*p) && p >= fullname) p--;
    *(p+1) = 0;
  }
  
  return result;
}

uint32_t get32(uint8_t *p) { return (*p) | (*(p+1) << 8) | (*(p+2) << 16) | (*(p+3) << 24); }

void write32(uint8_t *p, uint32_t v)
{
  *p = v & 0xff;
  *(p+1) = (v>>8) & 0xff;
  *(p+2) = (v>>16) & 0xff;
  *(p+3) = (v>>24) & 0xff;
}

int fput32(uint32_t v, FILE *f)
{
  for (int i = 0; i < 4; i++)
  {
    if (fputc(v & 0xff, f)==EOF) return EOF;
    v >>= 8;
  }
  return 0;
}



int iszlib(uint8_t *p)
{
  if ((*p) > 0x78 || (((*p)&0x0f) != 8)) return 0;
  return (((*p)<<8) | (*(p+1))) % 31 == 0;
}

