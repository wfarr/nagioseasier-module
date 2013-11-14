#ifndef NEZ_COMMANDS_H
#define NEZ_COMMANDS_H

typedef struct {
  const char* name;
  int(*handler)(int sd, char* object, char* rest);
}
nez_command_t;

#endif
