#ifndef NEZ_COMMANDS_H
#define NEZ_COMMANDS_H

typedef int(*nez_handler_t)(int sd, char* object, char* rest);

typedef struct {
  const char* name;
  nez_handler_t handler;
}
nez_command_t;

nez_handler_t nez_lookup_command(const char* cmd);

#endif
