#include <string.h>

#include <nagios/downtime.h>
#include <nagios/nagios.h>

#include "helper.h"
#include "commands.h"
#include "commands/acknowledgment.h"
#include "commands/check.h"
#include "commands/help.h"
#include "commands/notifications.h"
#include "commands/problems.h"
#include "commands/status.h"

static int
schedule_downtime_for_obj(int sd, const char* obj, unsigned long minutes, char* comment_data)
{

  host*    hst;
  service* svc;
  nez_find_host_or_service(obj, &hst, &svc);

  if (hst || svc) {

    int           typedowntime = (svc ? SERVICE_DOWNTIME : HOST_DOWNTIME);
    char*         hst_name     = (svc ? svc->host_name : hst->name);
    char*         svc_name     = (svc ? svc->description : NULL);
    time_t        entry_time   = time(NULL);
    char*         author       = "nagioseasier";
    time_t        start_time   = time(NULL);
    time_t        end_time     = 0L; /* only used for fixed downtime, TODO some other time */
    int           fixed        = 0;
    unsigned long triggered_by = 0L; /* assume triggered by no obj in system? */
    unsigned long duration     = minutes * 60L; /* assuming duration is seconds */
    unsigned long downtime_id  = 0L;


    end_time = start_time + duration;

    nsock_printf_nul(sd, "Setting %lu minutes of downtime for %s\n", minutes, obj);

    int retval = schedule_downtime(typedowntime,
      hst_name,
      svc_name,
      entry_time,
      author,
      comment_data,
      start_time,
      end_time,
      fixed,
      triggered_by,
      duration,
      &downtime_id);

    return (retval == OK ? 200 : 400);
  }

  nsock_printf_nul(sd, "NO HOST OR SERVICE FOUND FOR: %s", obj);
  return 404;
}

// COMMANDS

static int
nez_cmd_frenchman(int sd, char* object, char* rest)
{
  (void)object;
  (void)rest;
  nsock_printf_nul(sd, "yolochocinco!!!!!!\n");
  return 420;     // easter egg
}

static int
nez_cmd_schedule_downtime(int sd, char* object, char* rest)
{
  unsigned long minutes;
  char* comment_data;

  // assume the next argument is number of minutes
  if (rest) {
    if ((comment_data = strchr(rest, ' '))) {
      *(comment_data++) = 0;
    }

    minutes = strtoul(rest, NULL, 10);
  }

  minutes = (minutes > 1 ? minutes : 15L);

  return schedule_downtime_for_obj(sd, object, minutes, comment_data);
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
