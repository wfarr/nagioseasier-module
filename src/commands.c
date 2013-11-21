#include <string.h>

#include <nagios/downtime.h>
#include <nagios/nagios.h>

#include "helper.h"
#include "commands.h"
#include "commands/acknowledgment.h"
#include "commands/check.h"
#include "commands/downtime.h"
#include "commands/help.h"
#include "commands/notifications.h"
#include "commands/problems.h"
#include "commands/status.h"

// COMMANDS

static int
nez_cmd_frenchman(int sd, char* object, char* rest)
{
  (void)object;
  (void)rest;
  nsock_printf_nul(sd, "http://yolochocin.co/\n");
  return 420;     // easter egg
}

static int
unknown_command(int sd, char* object, char* rest)
{
  (void)object;
  (void)rest;
  nsock_printf_nul(sd, "UNKNOWN COMMAND\n");
  return 404;
}

static nez_command_t
commands[] = {
  { "help", nez_cmd_help },
  { "yolo", nez_cmd_frenchman },
  { "status", nez_cmd_status },
  { "check", nez_cmd_check },
  { "enable_notifications", nez_cmd_enable_notifications },
  { "disable_notifications", nez_cmd_disable_notifications },
  { "downtime", nez_cmd_schedule_downtime },
  { "acknowledge", nez_cmd_acknowledge },
  { "unacknowledge", nez_cmd_unacknowledge },
  { "problems", nez_cmd_problems },
  { "muted",  nez_cmd_show_muted },
};

nez_handler_t
nez_lookup_command(const char* cmd)
{

  if (cmd) {
    for (size_t i = 0; i < countof(commands); i++) {
      if (nez_string_equals(commands[i].name, cmd)) {
        return commands[i].handler;
      }
    }
  }

  return unknown_command;
}
