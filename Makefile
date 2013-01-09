.PHONY: all clean

CFLAGS=-O3 -Wall -Werror -g -std=gnu99
HEADERS=src/bfjit.h src/utils.h src/compiler.h

all:: bf

bf: bfjit.o utils.o driver.o compiler.o
	$(CC) $^ -o $@

%.o: src/%.c $(HEADERS)
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	$(RM) *.o
	$(RM) bf
