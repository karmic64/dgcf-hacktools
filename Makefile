ifdef COMSPEC
DOTEXE:=.exe
else
DOTEXE:=
endif


SRC_EXE := DFantasy/dfantasy.exe


CFLAGS := -s -Ofast -Wall

LIBS := -lpng -lz
# iconv must be explicitly linked on windows/mingw
ifdef COMSPEC
LIBS := $(LIBS) -liconv
endif

SRCS := pak.c grp.c
OUTS := $(SRCS:.c=$(DOTEXE))
DEPS := $(SRCS:.c=.d)


.PHONY: default all clean

default: all
all: $(OUTS) dfantasy-en.exe


clean:
	-$(RM) $(OUTS) $(DEPS)


imports.inc: $(SRC_EXE) dump-imports$(DOTEXE)
	./dump-imports$(DOTEXE) $< $@

dfantasy-en.exe: dfantasy-en.asm $(SRC_EXE) imports.inc
	fasm $< $@



%.d: %.c
	$(CC) -M -MG -MT $(<:.c=$(DOTEXE)) -MF $@ $<

-include $(DEPS)


%$(DOTEXE): %.c
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)
