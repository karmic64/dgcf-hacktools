ifdef COMSPEC
DOTEXE:=.exe
else
DOTEXE:=
endif


CFLAGS := -s -Ofast -Wall

LIBS := -lz -lpng

SRCS := pak.c
OUTS := $(SRCS:.c=$(DOTEXE))


.PHONY: default all clean

default: all
all: $(OUTS)


clean:
	rm $(OUTS)

%$(DOTEXE): %.c
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)
