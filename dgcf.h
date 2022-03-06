#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <iconv.h>


#define ZBUF_SIZE 0x80000


#define isslash(c) ((c)=='/' || (c)=='\\')
/* gets the actual filename part of a path name
   (e.g. "C:/Windows/notepad.exe" -> "notepad.exe")   */
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


/*
  Here's the problem:
  Filenames within a .pak are encoded in Shift-JIS.
  We need to convert these filenames to the encoding expected by the operating system.
  BUT, this differs per OS, AND file system!
  Linux ext4 partitions use byte strings encoded in UTF-8.
  Windows NTFS partitions use double-byte UCS-2LE strings.
  
  However, it's not as simple as just converting to UTF-8/UCS-2LE depending on the OS.
  Judging from my testing, fopen() on Linux always expects UTF-8 regardless of the file system.
  BUT, Windows converts byte strings to UCS-2 depending on the system's current code page...
  To avoid having to deal with getting and converting to the current code page, I use the Windows _wfopen() function, and convert the filename to wchars.
  Linux doesn't have this function, but it always expects UTF-8 so I can just use fopen().
  
  Worst part is that all this headache is due to only ONE file in data.pak called "se_refｌect01.wav" (notice the full-width lowercase L). ALL the other filenames use only ASCII characters...
  
  
  mkdir() is also a problem, on Windows this function takes only the directory name, but on Linux it also requires the permissions. Not supplying any permissions creates an unusable folder.
*/


/* per-platform defines */

#ifdef _WIN32

#include <wchar.h>
typedef wchar_t filenamechar_t;
#define dgcf_fopen_w(name) _wfopen((wchar_t*)name,L"wb")
#define dgcf_fopen_r(name) _wfopen((wchar_t*)name,L"rb")
#define dgcf_remove(name) _wremove((wchar_t*)name)

#include <direct.h>
#define dgcf_mkdir(name) mkdir(name)


#else

typedef char filenamechar_t;
#define dgcf_fopen_w(name) fopen((char*)name,"wb")
#define dgcf_fopen_r(name) fopen((char*)name,"rb")
#define dgcf_remove(name) remove((char*)name)

#include <sys/stat.h>
#define dgcf_mkdir(name) mkdir(name, S_IRWXU|S_IRWXG|S_IRWXO)


#endif


/* converts a Shift-JIS filename to the proper character set
   returns void* because the pointer should never be dereferenced outside of this file */
void *convertfilename(void *src)
{
  static filenamechar_t outbuf[256];
  size_t srclen = strlen(src);
  
  iconv_t ic = iconv_open(
#ifdef _WIN32
    "wchar_t"
#else
    "UTF-8"
#endif
    ,"SHIFT_JIS");
  if (!ic)
  {
    printf("iconv open failed: %s\n",strerror(errno));
    return NULL;
  }
  
  char *inptr = src;
  size_t inleft = srclen+1; /* convert the null too! */
  char *outptr = outbuf;
  size_t outleft = sizeof(outbuf);
  size_t status = iconv(ic,&inptr,&inleft,&outptr,&outleft);
  int en = errno;
  iconv_close(ic);
  
  if (status == (size_t)-1)
  {
    printf("iconv failed: %s\n",strerror(en));
    return NULL;
  }
  
  return outbuf;
}






/* endianness-independent multi-byte integer access */
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


/* returns 1 if p points to a zlib datastream */
int iszlib(uint8_t *p)
{
  if ((*p) > 0x78 || (((*p)&0x0f) != 8)) return 0;
  return (((*p)<<8) | (*(p+1))) % 31 == 0;
}

