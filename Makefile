CC = gcc
CFLAGS = -Wall -Wextra -g

OBJ = main.o tokenizer/tokenizer.o parser/parser.o

mshon: $(OBJ)
	$(CC) $(CFLAGS) -o mshon $(OBJ)

main.o: main.c tokenizer/tokenizer.h parser/parser.h
	$(CC) $(CFLAGS) -c main.c

parser/parser.o: parser/parser.c parser/parser.h tokenizer/tokenizer.h
	$(CC) $(CFLAGS) -c parser/parser.c -o parser/parser.o

tokenizer/tokenizer.o: tokenizer/tokenizer.c tokenizer/tokenizer.h
	$(CC) $(CFLAGS) -c tokenizer/tokenizer.c -o tokenizer/tokenizer.o

clean:
	rm -f $(OBJ) mshon
