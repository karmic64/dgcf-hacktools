#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <setjmp.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>

#include <zlib.h>
#include <png.h>

#include "dgcf.h"


/*

grp graphics format:

  dword - bits per pixel. the only used values seem to be:
    $08: indexed 8-bit
    $20: BGR0
  dword - always 0?
  dword - always 1?
  dword - bytes per row of image (width * bpp)
  4 dwords - always 0?
  dword - image width
  dword - image height
  $400 bytes - $100 colors in RGB0
    some images have this data filled, even if the actual image data is BGR0
  
  then the zlib-compressed image data, arranged row by row, left to right
  
  yes, the palette data is RGB, while the image is BGR

*/




enum
{
  FMT_RAW = 0,
  FMT_GRP,
  FMT_PNG,
};
const char *exts[] = {".raw",".grp",".png"};

int convert(char *name)
{
  char *shortname = getfilename(name);
  printf("Converting %s...",shortname);
  
  /* try to detect format */
  int infmt;
  int outfmt;
  struct stat st;
  if (stat(name, &st))
  {
    puts(strerror(errno));
    return 1;
  }
  FILE *inf = fopen(name, "rb");
  if (!inf)
  {
    puts(strerror(errno));
    return 1;
  }
  uint8_t header[0x42a];
  if (fread(header, 1, 0x42a, inf) != 0x42a)
  {
    if (ferror(inf))
    {
      fclose(inf);
      printf("Read error: %s\n", strerror(errno));
      return 1;
    }
    /* definitely not a grp */
    header[0x428] = 0xff;
  }
  
  if (!png_sig_cmp(header,0,8)) infmt = FMT_PNG;
  else if (iszlib(header+0x428)) infmt = FMT_GRP;
  else
  {
    puts("Unrecognized format");
    fclose(inf);
    return 1;
  }
  outfmt = infmt == FMT_PNG ? FMT_GRP : FMT_PNG;
  
  /* read image data */
#define CONVERT_READ_CLEAN() { free(image); free(rows); }
#define CONVERT_READ_FAIL() { CONVERT_READ_CLEAN(); return 1; }
  uint32_t bpp;
  uint32_t width;
  uint32_t height;
  png_color palette[256];
  uint8_t *image = NULL;
  size_t imagesize = 0;
  uint8_t **rows = NULL;
  
  void initrows()
  {
    rows = malloc(height * sizeof(*rows));
    for (size_t i = 0; i < height; i++)
    {
      rows[i] = image + (i*width*(bpp/8));
    }
  }
  
  switch (infmt)
  {
    case FMT_GRP:
    {
      bpp = get32(header+0x00);
      width = get32(header+0x20);
      height = get32(header+0x24);
      if (bpp != 0x08 && bpp != 0x20)
      {
        puts("Invalid bits per pixel");
        fclose(inf);
        CONVERT_READ_FAIL();
      }
      if (!width)
      {
        puts("Zero width");
        fclose(inf);
        CONVERT_READ_FAIL();
      }
      if (!height)
      {
        puts("Zero height");
        fclose(inf);
        CONVERT_READ_FAIL();
      }
      for (int i = 0; i < 0x100; i++)
      {
        palette[i].red = header[0x28 + (i*4) + 0];
        palette[i].green = header[0x28 + (i*4) + 1];
        palette[i].blue = header[0x28 + (i*4) + 2];
      }
      
#define CONVERT_READ_GRP_FAIL() { free(inbuf); CONVERT_READ_FAIL(); }
      size_t inbufsize = st.st_size-0x428;
      uint8_t *inbuf = malloc(inbufsize);
      memcpy(inbuf, header+0x428, 2);
      if (fread(inbuf+2, 1, inbufsize-2, inf) != inbufsize-2)
      {
        printf("Read error: %s\n", strerror(errno));
        fclose(inf);
        CONVERT_READ_GRP_FAIL();
      }
      fclose(inf);
      
      z_stream zs;
      zs.next_in = inbuf;
      zs.avail_in = inbufsize;
      zs.zalloc = NULL;
      zs.zfree = NULL;
      zs.opaque = NULL;
      if (inflateInit(&zs) != Z_OK)
      {
        printf("zlib init error: %s\n", zs.msg);
        CONVERT_READ_GRP_FAIL();
      }
      size_t imagemax = 0x10000;
      image = malloc(imagemax);
      zs.next_out = image;
      zs.avail_out = imagemax;
      while (1)
      {
        int status = inflate(&zs, Z_NO_FLUSH);
        if (status < 0)
        {
          printf("Decompression error: %s\n", zs.msg);
          inflateEnd(&zs);
          CONVERT_READ_GRP_FAIL();
        }
        if (status == Z_STREAM_END) break;
        if (!zs.avail_out)
        {
          size_t o = imagemax;
          imagemax *= 2;
          image = realloc(image, imagemax);
          zs.next_out = image+o;
          zs.avail_out = imagemax-o;
        }
      }
      imagesize = zs.total_out;
      inflateEnd(&zs);
      
      
      
      free(inbuf);
      
      initrows();
      break;
    }
    case FMT_PNG:
    {
      png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
      if (!png_ptr)
      {
        puts("PNG init error");
        fclose(inf);
        CONVERT_READ_FAIL();
      }
      png_infop info_ptr = png_create_info_struct(png_ptr);
      if (!info_ptr)
      {
        puts("PNG info init error");
        png_destroy_write_struct(&png_ptr,NULL);
        fclose(inf);
        CONVERT_READ_FAIL();
      }
      
      if (setjmp(png_jmpbuf(png_ptr)))
      {
        png_destroy_write_struct(&png_ptr,&info_ptr);
        fclose(inf);
        CONVERT_READ_FAIL();
      }
      
      fseek(inf,8,SEEK_SET);
      png_init_io(png_ptr,inf);
      png_set_sig_bytes(png_ptr,8);
      
      png_read_info(png_ptr,info_ptr);
      width = png_get_image_width(png_ptr,info_ptr);
      height = png_get_image_height(png_ptr,info_ptr);
      int colortype = png_get_color_type(png_ptr,info_ptr);
      if (!(colortype & PNG_COLOR_MASK_COLOR))
      {
        puts("Grayscale image");
        longjmp(png_jmpbuf(png_ptr),1);
      }
      if (colortype & PNG_COLOR_MASK_ALPHA)
      {
        puts("Image has alpha channel");
        longjmp(png_jmpbuf(png_ptr),1);
      }
      if (colortype & PNG_COLOR_MASK_PALETTE) bpp = 0x08;
      else bpp = 0x20;
      png_colorp pngpal;
      int palsize;
      png_get_PLTE(png_ptr,info_ptr,&pngpal,&palsize);
      if (palsize > 0x100)
      {
        puts("Palette too large");
        longjmp(png_jmpbuf(png_ptr),1);
      }
      for (int i = 0; i < 0x100; i++)
      {
        if (i >= palsize)
        {
          palette[i].red = 0;
          palette[i].green = 0;
          palette[i].blue = 0;
        }
        else
        {
          palette[i].red = pngpal[i].red;
          palette[i].green = pngpal[i].green;
          palette[i].blue = pngpal[i].blue;
        }
      }
      
      png_set_packing(png_ptr);
      if (bpp == 0x20)
      {
        png_set_bgr(png_ptr);
        png_set_filler(png_ptr,0,PNG_FILLER_AFTER);
      }
      
      imagesize = height*width*4;
      image = malloc(imagesize);
      initrows();
      png_read_image(png_ptr,rows);
      png_read_end(png_ptr,NULL);
      png_destroy_read_struct(&png_ptr,&info_ptr,NULL);
      fclose(inf);
      
      break;
    }
  }
  
  
  
  
  /* create output file */
#define CONVERT_WRITE_CLEAN() { CONVERT_READ_CLEAN(); fclose(of); }
#define CONVERT_WRITE_FAIL() { CONVERT_WRITE_CLEAN(); remove(outname); return 1; }
  int nlen = strlen(name);
  char outname[nlen+5];
  memcpy(outname, name, nlen);
  memcpy(outname+nlen, exts[outfmt], 5);
  
  FILE *of = fopen(outname, "wb");
  switch (outfmt)
  {
    case FMT_RAW:
    {
      if (imagesize != fwrite(image, 1, imagesize, of))
      {
        printf("Write error: %s\n", strerror(errno));
        CONVERT_WRITE_FAIL();
      }
      break;
    }
    case FMT_GRP:
    {
      fput32(bpp,of);
      fput32(0,of);
      fput32(1,of);
      fput32(width*(bpp/8),of);
      for (int i = 0; i < 0x10; i++) fputc(0,of);
      fput32(width,of);
      fput32(height,of);
      
      for (int i = 0; i < 0x100; i++)
      {
        fputc(palette[i].red,of);
        fputc(palette[i].green,of);
        fputc(palette[i].blue,of);
        fputc(0,of);
      }
      
      uint8_t outbuf[ZBUF_SIZE];
      z_stream zs;
      zs.next_in = image;
      zs.avail_in = imagesize;
      zs.zalloc = NULL;
      zs.zfree = NULL;
      zs.opaque = NULL;
      int status = deflateInit(&zs, Z_BEST_COMPRESSION);
      if (status != Z_OK)
      {
        printf("zlib init error: %s\n", zs.msg);
        CONVERT_WRITE_FAIL();
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
          CONVERT_WRITE_FAIL();
        }
        size_t s = ZBUF_SIZE-zs.avail_out;
        if (s != fwrite(outbuf, 1, s, of))
        {
          printf("Write error: %s\n", strerror(errno));
          deflateEnd(&zs);
          CONVERT_WRITE_FAIL();
        }
        if (status == Z_STREAM_END) break;
      }
      
      deflateEnd(&zs);
      break;
    }
    case FMT_PNG:
    {
      png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
      if (!png_ptr)
      {
        puts("PNG init error");
        CONVERT_WRITE_FAIL();
      }
      png_infop info_ptr = png_create_info_struct(png_ptr);
      if (!info_ptr)
      {
        puts("PNG info init error");
        png_destroy_write_struct(&png_ptr,NULL);
        CONVERT_WRITE_FAIL();
      }
      
      if (setjmp(png_jmpbuf(png_ptr)))
      {
        png_destroy_write_struct(&png_ptr,&info_ptr);
        CONVERT_WRITE_FAIL();
      }
      
      png_init_io(png_ptr,of);
      // this is very slow, use default compression
      // png_set_compression_level(png_ptr,Z_BEST_COMPRESSION);
      png_set_IHDR(png_ptr,info_ptr, width,height,8,bpp==0x20 ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_PALETTE,
                    PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
      png_set_PLTE(png_ptr,info_ptr, palette,256);
      
      png_write_info(png_ptr,info_ptr);
      if (bpp == 0x20)
      {
        png_set_filler(png_ptr,0,PNG_FILLER_AFTER);
        png_set_bgr(png_ptr);
      }
      
      png_write_image(png_ptr,rows);
      png_write_end(png_ptr, info_ptr);
      png_destroy_write_struct(&png_ptr, &info_ptr);
      break;
    }
  }
  
  
  CONVERT_WRITE_CLEAN();
  puts("OK");
  fflush(NULL);
  return 0;
}




int main(int argc, char *argv[])
{
  if (argc == 1)
  {
    printf(
      "Di Gi Charat Fantasy graphics converter by karmic\n"
      "\n"
      "Usage:\n"
      "  grp files...\n"
      "\n"
      "Source files can be in either .grp or .png format.\n"
      );
    return EXIT_FAILURE;
  }
  else
  {
    unsigned errors = 0;
    for (int i = 1; i < argc; i++)
    {
      printf("(%i/%i) ",i, argc-1);
      if (convert(argv[i])) errors++;
    }
    printf("Converting finished");
    if (errors) printf(" with %u errors",errors);
    puts(".");
  }
}