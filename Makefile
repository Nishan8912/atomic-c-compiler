CFLAGS = -I. -Wall -Wno-unused-function -g
CC = gcc
LEX = flex
YACC = yacc

all: compiler7

compiler7: lex.yy.o y.tab.o symtable.o astree.o
	$(CC) -o $@ $^ $(CFLAGS)

y.tab.c y.tab.h: parser.y
	$(YACC) -d parser.y

lex.yy.c: scanner.l y.tab.h
	$(LEX) scanner.l

%.o: %.c
	$(CC) $(CFLAGS) -c $<

# === Test Targets ===

assembly: compiler7
	./compiler7 test.j

test: assembly
	gcc -z noexecstack -no-pie -o test test.s
	./test

memcheck: compiler7
	valgrind --leak-check=full \
         --track-origins=yes \
         --show-leak-kinds=all \
         --error-exitcode=1 \
         --log-file=valgrind.log \
         ./compiler7 test.j
	@echo "Valgrind report saved to valgrind.log"

clean:
	rm -f lex.yy.c y.tab.c y.tab.h *.o compiler7 test.s a.out test vgcore.36632 valgrind.log

.PHONY: all clean test assembly memcheck
