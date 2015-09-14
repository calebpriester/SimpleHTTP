
CC=clang
CFLAGS=-Wall -g

BINS=simhttp simget

all: $(BINS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $< DieWithMessage.c

clean:
	rm -f ${BINS}
