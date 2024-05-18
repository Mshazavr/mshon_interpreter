CC = gcc
CFLAGS = -I./include -Wall -Wextra -g -Wno-missing-field-initializers
VPATH = include

OBJ = build/main.o build/tokenizer.o build/parser.o build/hash_table.o build/stack.o build/evaluator.o
OBJ_T = build/test.o build/tokenizer.o build/parser.o build/hash_table.o build/stack.o build/evaluator.o

mshon: $(OBJ)
	$(CC) $(CFLAGS) -o bin/mshon $(OBJ)

test: $(OBJ_T)
	$(CC) $(CFLAGS) -o bin/test $(OBJ_T)

build/main.o: src/main.c include/tokenizer.h include/parser.h include/evaluator.h
	$(CC) $(CFLAGS) -c src/main.c -o build/main.o

build/test.o: tests/runner.c include/tokenizer.h include/parser.h include/evaluator.h
	$(CC) $(CFLAGS) -c tests/runner.c -o build/test.o

build/parser.o: src/parser.c include/parser.h include/tokenizer.h
	$(CC) $(CFLAGS) -c src/parser.c -o build/parser.o

build/tokenizer.o: src/tokenizer.c include/tokenizer.h
	$(CC) $(CFLAGS) -c src/tokenizer.c -o build/tokenizer.o

build/hash_table.o: src/hash_table.c include/hash_table.h 
	$(CC) $(CFLAGS) -c src/hash_table.c -o build/hash_table.o 

build/stack.o: src/stack.c include/stack.h 
	$(CC) $(CFLAGS) -c src/stack.c -o build/stack.o 

build/evaluator.o: src/evaluator.c include/evaluator.h 
	$(CC) $(CFLAGS) -c src/evaluator.c -o build/evaluator.o 

clean:
	rm -f $(OBJ) $(OBJ_T) bin/mshon bin/test
