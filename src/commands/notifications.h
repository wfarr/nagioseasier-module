#ifndef NEZ_COMMANDS_NOTIFICATIONS_H
#define NEZ_COMMANDS_NOTIFICATIONS_H

int nez_cmd_enable_notifications(int sd, char* object, char* rest);
int nez_cmd_disable_notifications(int sd, char* object, char* rest);

int nez_cmd_show_muted(int sd, char* object, char* rest);

#endif
