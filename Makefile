CFLAGS=-Wall -Wextra -pedantic -std=c99 -Werror -D_XOPEN_SOURCE=500 -fPIC -g

SONAME=nagioseasier.so
OBJECTS=src/nagioseasier.o

all: $(SONAME)

$(SONAME): $(OBJECTS)
	$(CC) -shared $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm -f $(SONAME) $(OBJECTS)
