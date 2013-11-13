CFLAGS=-Wall -Wextra -pedantic -std=c99 -Werror -D_XOPEN_SOURCE

all: nagioseasier.o

clean:
	rm -f nagioseasier.o
