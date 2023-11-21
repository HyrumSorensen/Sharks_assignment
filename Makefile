all: sharks

sharks: sharks.c
	gcc -Wall -Wextra -Wpedantic -Werror -std=c99 -pthread sharks.c -o sharks

clean:
	rm -f sharks sharks.o