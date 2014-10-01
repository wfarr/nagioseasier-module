CFLAGS=-Wall -Wextra -pedantic -std=c99 -Werror -D_XOPEN_SOURCE=500 -fPIC -g $(LIB) $(INC)

SONAME=nagioseasier.so
OBJECTS=\
	src/commands.o \
	src/commands/acknowledgment.o \
	src/commands/check.o \
	src/commands/downtime.o \
	src/commands/help.o \
	src/commands/notifications.o \
	src/commands/problems.o \
	src/commands/stats.o \
	src/commands/status.o \
	src/helper.o \
	src/nagioseasier.o \

all: $(SONAME)

$(SONAME): $(OBJECTS)
	$(CC) -shared $(CFLAGS) $(LDFLAGS) -o $@ $^

.PHONY: clean doctor

clean:
	rm -f $(SONAME) $(OBJECTS)

doctor: $(SONAME)
	SONAME=$(SONAME) ruby script/doctor.rb
