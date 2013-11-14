#ifndef NEZ_COMMANDS_ACKNOWLEDGMENT_H
#define NEZ_COMMANDS_ACKNOWLEDGMENT_H

int nez_cmd_acknowledge(int sd, char* object, char* rest);
int nez_cmd_unacknowledge(int sd, char* object, char* rest);

#endif
