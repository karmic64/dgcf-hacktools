#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/stat.h>



uint16_t get16(uint8_t *p) { return (*p) | (*(p+1))<<8; }
uint32_t get32(uint8_t *p) { return (*p) | (*(p+1))<<8 | (*(p+2))<<16 | (*(p+3))<<24; }



typedef struct {
	char name[8];
	
	uint32_t rva;
	uint32_t vsize;
	uint32_t vend;
	
	uint32_t offs;
	uint32_t size;
	uint32_t end;
} section_t;



uint32_t image_base;

#define MAX_SECTIONS 96
unsigned sectioncnt = 0;
section_t sections[MAX_SECTIONS];


FILE *f = NULL;
char *outname = NULL;



void __attribute__((noreturn)) fail()
{
	if (f) fclose(f);
	remove(outname);
	exit(EXIT_FAILURE);
}


uint32_t rva2file(uint32_t rva)
{
	for (int i = 0; i < sectioncnt; i++) 
	{
		if (rva >= sections[i].rva && rva < sections[i].vend && rva < sections[i].rva+sections[i].size)
			return (rva - sections[i].rva) + sections[i].offs;
	}
	printf("No file offset associated with RVA $%X\n", rva);
	fail();
}



int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		puts(
			"This tool dumps the addresses of the imports of a PE executable to an assembly include file.\n"
			"\n"
			"Usage:\n"
			"  dump-imports inname outname"
		);
		return EXIT_FAILURE;
	}
	
	
	const char *inname = argv[1];
	outname = argv[2];	
	
	struct stat st;
	if (stat(inname, &st))
	{
		printf("Can't stat %s: %s\n", inname, strerror(errno));
		return EXIT_FAILURE;
	}
	size_t infsize = st.st_size;
	if (infsize < 0x40)
	{
		printf("Infile is too small (%lu bytes). Can't get PE header offset.\n",infsize);
		return EXIT_FAILURE;
	}
	
	
	int en;
	f = fopen(inname,"rb");
	if (!f)
	{
		printf("Can't open %s: %s\n", inname, strerror(errno));
		return EXIT_FAILURE;
	}
	uint8_t *inbuf = malloc(infsize);
	size_t inread = fread(inbuf,1,infsize,f);
	en = errno;
	fclose(f);
	f = NULL;
	if (inread != infsize)
	{
		printf("Error while reading %s: %s\n", inname, strerror(en));
		return EXIT_FAILURE;
	}
	
	
	uint32_t peoffs = get32(inbuf+0x3c);
	if (peoffs >= infsize-(24))
	{
		printf("Invalid PE offset $%x\n",peoffs);
		return EXIT_FAILURE;
	}
	if (memcmp("PE\0",&inbuf[peoffs],4))
	{
		printf("Not a PE\n");
		return EXIT_FAILURE;
	}
	
	uint8_t *coffheader = &inbuf[peoffs+4];
	uint8_t *optheader = &coffheader[20];
	
	sectioncnt = get16(coffheader+2);
	if (!sectioncnt || sectioncnt > MAX_SECTIONS)
	{
		printf("Invalid section count %u\n",sectioncnt);
		return EXIT_FAILURE;
	}
	
	uint32_t optheadersize = get16(coffheader+16);
	uint32_t minoptheadersize = 224;
	if (optheadersize < minoptheadersize)
	{
		printf("Invalid optional header size %u (should be at least %u)\n",optheadersize,minoptheadersize);
		return EXIT_FAILURE;
	}
	
	uint16_t optmagic = get16(optheader);
	if (optmagic != 0x10b)
	{
		printf("Bad/unsupported optional header magic %04X\n",optmagic);
		return EXIT_FAILURE;
	}
	image_base = get32(optheader+28);
	printf("PE image base is $%X\n", image_base);
	
	uint8_t *sectiontable = &optheader[optheadersize];
	printf("PE file has %u sections\n",sectioncnt);
	uint32_t maxvend = 0;
	uint32_t maxend = 0;
	for (int i = 0; i < sectioncnt; i++)
	{
		uint8_t *thissection = &sectiontable[i*40];
		
		memcpy(sections[i].name, thissection, 8);
		
		sections[i].vsize = get32(thissection+8);
		sections[i].rva = get32(thissection+12);
		sections[i].vend = sections[i].rva+sections[i].vsize;
		
		sections[i].size = get32(thissection+16);
		sections[i].offs = get32(thissection+20);
		sections[i].end = sections[i].offs+sections[i].size;
		
		if (sections[i].vend > maxvend) maxvend = sections[i].vend;
		if (sections[i].end > maxend) maxend = sections[i].end;
		
		printf("Section #%i: \"%.8s\", RVA $%X-$%X, File $%X-$%X\n",
			i+1, sections[i].name,
			sections[i].rva, sections[i].vend-1,
			sections[i].offs, sections[i].end-1
		);
	}
	
	if (maxend > infsize)
	{
		printf("Maximum file offset $%X is greater than the actual file size $%lX\n",maxend,infsize);
		return EXIT_FAILURE;
	}
	
	
	uint32_t importtablerva = get32(optheader+104);
	uint32_t importtablesize = get32(optheader+108);
	
	uint32_t importtableoffs = rva2file(importtablerva);
	f = fopen(outname,"w");
	if (!f)
	{
		printf("Couldn't open %s: %s\n", outname, strerror(errno));
		return EXIT_FAILURE;
	}
	
	
	unsigned importoffs = 0;
	while (importoffs+20 <= importtablesize)
	{
		uint8_t *importtableent = &inbuf[importtableoffs+importoffs];
		/* all nulls marks the end of the import table */
		int i = 0;
		for ( ; i < 20; i++)
			if (importtableent[i]) break;
		if (i == 20) break;
		
		char *importname = (char*)&inbuf[rva2file(get32(importtableent+12))];
		uint8_t *importlookup = &inbuf[rva2file(get32(importtableent+0))];
		uint32_t importaddressrva = get32(importtableent+16);
		
		printf("\nReading %s imports:\n",importname);
		fprintf(f, "\n;;;;; %s ;;;;;\n", importname);
		unsigned li = 0;
		while (1)
		{
			uint32_t namerva = get32(importlookup + li*4);
			
			if (!namerva) break;
			
			if (namerva & 0x80000000)
			{
				printf("Ignoring ordinal import $%X, RVA $%X\n",namerva,importaddressrva+(li*4));
				fprintf(f, "; (ignored ordinal import $%X, VA $%X)\n",namerva,importaddressrva+(li*4)+image_base);
			}
			else
			{
				char *name = (char*)&inbuf[rva2file(namerva)+2];
				printf("\"%s\", RVA $%X\n",name,importaddressrva+(li*4));
				fprintf(f, "%s equ $%X\n",name,importaddressrva+(li*4)+image_base);
			}
			
			li++;
		}
		
		importoffs += 20;
	}
	
	fclose(f);
	
	return EXIT_SUCCESS;
	
}

