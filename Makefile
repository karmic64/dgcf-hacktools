ifdef COMSPEC
DOTEXE:=.exe
else
DOTEXE:=
endif


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
all: $(OUTS)


clean:
	-$(RM) $(OUTS) $(DEPS)



%.d: %.c
	$(CC) -M -MG -MT $(<:.c=$(DOTEXE)) -MF $@ $<

-include $(DEPS)


%$(DOTEXE): %.c
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)
