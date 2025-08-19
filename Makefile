NAME:= load_csv

CFLAGS:= -ggdb3 -fsanitize=address -O3

all:
	cc $(CFLAGS) main.c -o $(NAME)

run: all
	./$(NAME)

clean:
	rm -f $(NAME)

re: clean all