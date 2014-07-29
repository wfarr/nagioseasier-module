#ifndef NEZ_COMMANDS_DOWNTIME_H
#define NEZ_COMMANDS_DOWNTIME_H

int nez_cmd_schedule_fixed_downtime(int sd, char* object, char* rest);
int nez_cmd_schedule_flex_downtime(int sd, char* object, char* rest);

#endif
