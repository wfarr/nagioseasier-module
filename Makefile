CFLAGS=-Wall -Wextra -pedantic -std=c99 -Werror -D_XOPEN_SOURCE=500 -fPIC -g

SONAME=nagioseasier.so
OBJECTS=src/nagioseasier.o src/helper.o src/commands.o src/commands/help.o src/commands/status.o src/commands/notifications.o src/commands/check.o src/commands/acknowledgment.o src/commands/problems.o src/commands/downtime.o

all: $(SONAME)

$(SONAME): $(OBJECTS)
	$(CC) -shared $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm -f $(SONAME) $(OBJECTS)
