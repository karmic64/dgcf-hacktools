#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>

#include <zlib.h>

#include "dgcf.h"


/*

pak archive format:

  for each file:
    $30 bytes - 0-padded filename
    dword - offset of file data relative to start of file data area
    dword - .....and once more
    dword - file size (after compression)
    dword - compression type
      0: none
      2: zlib
  and then the file data area
  
  the first "file" entry in the toc is always called "DATA$TOP"
  instead of file data size, the value is the amount of files in the archive,
  INCLUDING the fake first file
  rest of the values are kept as 0

note that the files "data01.pak"-"data17.pak" in the dir are not archives but
are actually just renamed MPEG movies

the game does not appear to work correctly if files in data.pak are compressed

*/







#define UNP_FAIL() { fclose(inf); puts("Extraction failed."); return EXIT_FAILURE; }
int unpack(char *pakname)
{
  printf("Opening %s...", pakname);
  FILE *inf = fopen(pakname, "rb");
  if (!inf)
  {
    puts(strerror(errno));
    puts("Extraction failed.");
    return EXIT_FAILURE;
  }
  puts("OK");
  
  printf("Reading %s...", pakname);
  uint8_t firstent[0x40];
  fread(firstent, 1, 0x40, inf);
  if (memcmp("DATA$TOP", firstent, 8))
  {
    puts("Invalid archive signature");
    UNP_FAIL();
  }
  uint32_t filec = get32(firstent+0x38)-1;
  printf("OK, %u files\n", filec);
  
  
  
  char dirname[strlen(pakname)+5];
  sprintf(dirname, "%s-out", getfilename(pakname));
  
  {
    int me = 0;
    int ce = 0;
    if (mkdir(dirname)) me = errno;
    if (chdir(dirname)) ce = errno;
    
    if (ce)
    {
      if (me) printf("Couldn't create output directory: %s\n", strerror(me));
      else printf("Couldn't change to output directory: %s\n", strerror(ce));
      UNP_FAIL();
    }
  }
  
  
  size_t dataoffs = (filec+1)*0x40;
  size_t tocsize = dataoffs-0x40;
  unsigned errors = 0;
  
  uint8_t toc[tocsize];
  if (tocsize != fread(toc, 1, tocsize, inf))
  {
    if (feof(inf)) printf("Unexpected end-of-file while reading TOC\n");
    else printf("Error while reading TOC: %s\n", strerror(errno));
    UNP_FAIL();
  }
  
  
  for (unsigned f = 0; f < filec; f++)
  {
    uint8_t *ent = &toc[f*0x40];
    
    printf("(%u/%u) Extracting %.48s...", f+1,filec, ent);
    
    uint32_t foffs1 = get32(&ent[0x30]);
    uint32_t foffs2 = get32(&ent[0x34]);
    uint32_t fsize = get32(&ent[0x38]);
    uint32_t compmode = get32(&ent[0x3c]);
    
    if (foffs1 != foffs2)
    {
      puts("Inconsistent file offset");
      errors++;
    }
    else if (!(*ent))
    {
      puts("Blank filename");
      errors++;
    }
    else if (!fsize)
    {
      puts("Zero filesize");
      errors++;
    }
    else if (compmode != 0 && compmode != 2)
    {
      puts("Invalid compression type");
      errors++;
    }
    else
    {
#define UNP_FILE_FAIL() { fclose(of); remove((char*)ent); errors++; goto unp_file_next; }
      FILE *of = fopen((char*)ent, "wb");
      if (!of)
      {
        printf("Couldn't open for writing: %s\n", strerror(errno));
        errors++;
        goto unp_file_next;
      }
      fseek(inf, foffs1+dataoffs, SEEK_SET);
      
      if (compmode == 0)
      {
        for (size_t i = 0; i < fsize; i++)
        {
          int c = fgetc(inf);
          if (c==EOF)
          {
            if (feof(inf)) printf("Unexpected end-of-file\n");
            else printf("Read error: %s\n", strerror(errno));
            UNP_FILE_FAIL();
          }
          c = fputc(c,of);
          if (c==EOF)
          {
            printf("Write error: %s\n", strerror(errno));
            UNP_FILE_FAIL();
          }
        }
      }
      else if (compmode == 2)
      {
        uint8_t indata[fsize];
        if (fsize != fread(indata, 1, fsize, inf))
        {
          if (feof(inf)) printf("Unexpected end-of-file\n");
          else printf("Read error: %s\n", strerror(errno));
          UNP_FILE_FAIL();
        }
        
        z_stream zs;
        zs.next_in = indata;
        zs.avail_in = fsize;
        zs.zalloc = NULL;
        zs.zfree = NULL;
        zs.opaque = NULL;
        int status = inflateInit(&zs);
        if (status != Z_OK)
        {
          printf("zlib init error: %s\n", zs.msg);
          UNP_FILE_FAIL();
        }
        
        uint8_t outbuf[ZBUF_SIZE];
        while (zs.avail_in)
        {
          zs.next_out = outbuf;
          zs.avail_out = ZBUF_SIZE;
          status = inflate(&zs, Z_NO_FLUSH);
          if (status < 0)
          {
            printf("Decompression error: %s\n", zs.msg);
            inflateEnd(&zs);
            UNP_FILE_FAIL();
          }
          size_t s = ZBUF_SIZE-zs.avail_out;
          if (s != fwrite(outbuf, 1, s, of))
          {
            printf("Write error: %s\n", strerror(errno));
            inflateEnd(&zs);
            UNP_FILE_FAIL();
          }
          else if (status == Z_STREAM_END && zs.avail_in)
          {
            printf("(warning: trailing data) ");
            break;
          }
        }
        
        inflateEnd(&zs);
      }
      
      fclose(of);
      
      puts("OK");
      
unp_file_next:
      ;
    }
  }
  
  
  
  fclose(inf);
  printf("Extraction finished with ");
  if (!errors) printf("no errors.\n");
  else if (errors == 1) printf("1 error.\n");
  else printf("%u errors.\n", errors);
  
  return EXIT_SUCCESS;
  
}






#define PACK_FAIL() { puts("Packing failed."); chdir(initialcwd); fclose(of); remove(pakname); free(initialcwd); free(toc); return EXIT_FAILURE; }
int pack(char *pakname, char *dn, int compmode)
{
  if (compmode != 0 && compmode != 2)
  {
    printf("Unsupported compression mode %i\n", compmode);
    return EXIT_FAILURE;
  }
  
  printf("Opening %s...", pakname);
  FILE *of = fopen(pakname, "wb");
  if (!of)
  {
    puts(strerror(errno));
    return EXIT_FAILURE;
  }
  puts("OK");
  
  
  /* count files */
  unsigned files = 0;
  unsigned warnings = 0;
  char *initialcwd = getcwd(NULL, 0);
  struct stat st;
  
  struct tocent {
    char name[0x30];
    uint32_t offs;
    uint32_t csize;
  };
  
  struct tocent *toc = NULL;
  unsigned tocmax = 0;
  
  size_t maxinsize = 0;
  
  printf("Scanning %s...", dn);
  if (!chdir(dn))
  {
    putchar('\n');
    DIR *dir = opendir(".");
    struct dirent *de;
    while ((de = readdir(dir)) != NULL)
    {
      char *n = de->d_name;
      if (!strcmp(n,".")) continue;
      if (!strcmp(n,"..")) continue;
      size_t nlen = strlen(n);
      
      if (nlen >= 0x30)
      {
        printf("WARNING: Skipping long filename %s\n", n);
        warnings++;
        continue;
      }
      
      stat(n, &st);
      if (!S_ISREG(st.st_mode))
      {
        printf("WARNING: Skipping irregular file %s\n", n);
        warnings++;
        continue;
      }
      
      if (files == tocmax)
      {
        if (tocmax) tocmax *= 2;
        else tocmax = 0x200;
        toc = realloc(toc, tocmax*sizeof(*toc));
      }
      memcpy(toc[files].name,n,nlen);
      memset(toc[files].name+nlen,0,0x30-nlen);
      if (st.st_size > maxinsize) maxinsize = st.st_size;
      
      files++;
    }
    closedir(dir);
  }
  else
  {
    puts(strerror(errno));
    PACK_FAIL();
  }
  
  printf("%u files will be packed.\n", files);
  
  
  /* pack files */
  uint32_t curoffs = 0;
  uint8_t *inbuf = malloc(maxinsize);
  
  fseek(of, (files+1)*0x40, SEEK_SET);
  
#define PACK_FILE_FAIL() { free(inbuf); PACK_FAIL(); }
  for (unsigned f = 0; f < files; f++)
  {
    toc[f].offs = curoffs;
    char *n = toc[f].name;
    printf("(%u/%u) Packing %s...", f+1, files, n);
    FILE *inf = fopen(n,"rb");
    if (!inf)
    {
      puts(strerror(errno));
      PACK_FILE_FAIL();
    }
    size_t size = fread(inbuf, 1, maxinsize, inf);
    int inerr = ferror(inf);
    int inen = errno;
    fclose(inf);
    
    if (inerr)
    {
      printf("Read error: %s\n", strerror(inen));
      PACK_FILE_FAIL();
    }
    
    if (compmode == 0)
    {
      if (size != fwrite(inbuf, 1, size, of))
      {
        printf("Write error: %s\n", strerror(inen));
        PACK_FILE_FAIL();
      }
      puts("OK");
      
      toc[f].csize = size;
      curoffs += size;
    }
    else if (compmode == 2)
    {
      uint8_t outbuf[ZBUF_SIZE];
      
      z_stream zs;
      zs.next_in = inbuf;
      zs.avail_in = size;
      zs.zalloc = NULL;
      zs.zfree = NULL;
      zs.opaque = NULL;
      int status = deflateInit(&zs, Z_BEST_COMPRESSION);
      if (status != Z_OK)
      {
        printf("zlib init error: %s\n", zs.msg);
        PACK_FILE_FAIL();
      }
      
      while (1)
      {
        zs.next_out = outbuf;
        zs.avail_out = ZBUF_SIZE;
        int status = deflate(&zs, zs.avail_in ? Z_NO_FLUSH : Z_FINISH);
        if (status < 0)
        {
          printf("Compression error: %s\n", zs.msg);
          deflateEnd(&zs);
          PACK_FILE_FAIL();
        }
        size_t s = ZBUF_SIZE-zs.avail_out;
        toc[f].csize += s;
        curoffs += s;
        if (s != fwrite(outbuf, 1, s, of))
        {
          printf("Write error: %s\n", strerror(errno));
          deflateEnd(&zs);
          PACK_FILE_FAIL();
        }
        if (status == Z_STREAM_END) break;
      }
      
      deflateEnd(&zs);
      puts("OK");
    }
  }
  
  free(inbuf);
  
  
  /* write toc */
  fseek(of,0,SEEK_SET);
  fputs("DATA$TOP",of);
  for (int i = 0; i < 0x30; i++) fputc(0,of);
  fput32(files+1,of);
  fput32(0,of);
  
  for (unsigned f = 0; f < files; f++)
  {
    fwrite(toc[f].name, 1, 0x30, of);
    fput32(toc[f].offs, of);
    fput32(toc[f].offs, of);
    fput32(toc[f].csize, of);
    fput32(compmode, of);
  }
  
  
  
  puts("Packing finished.");
  
  if (warnings)
  {
    if (warnings == 1) printf("1 file was");
    else printf("%u files were", warnings);
    puts(" skipped.");
  }
  
  free(initialcwd);
  free(toc);
  
  return EXIT_SUCCESS;
}






int main(int argc, char *argv[])
{
  if (argc == 2)
  {
    return unpack(argv[1]);
  }
  else if (argc == 3)
  {
    return pack(argv[1], argv[2], 0);
  }
  else if (argc == 4)
  {
    int c = atoi(argv[3]);
    return pack(argv[1], argv[2], c);
  }
  else
  {
    printf(
      "Di Gi Charat Fantasy (un)packer by karmic\n"
      "\n"
      "Usage:\n"
      "    pack mode:  pak outarchive dir [compmode]\n"
      "  unpack mode:  pak inarchive\n"
      );
    return EXIT_FAILURE;
  }
}