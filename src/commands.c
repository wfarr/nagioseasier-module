#include <string.h>

#include <nagios/nagios.h>

#include "commands.h"

static int
display_help(int sd)
{
  nsock_printf_nul(sd, "Query handler for actually doing useful shit with this socket.\n"
       "Available commands:\n"
       "  status                  Display the status of a host or service\n"
       "  enable_notifications    Enable notifications for a host or host-service\n"
       "  disable_notifications   Disable notifications for a host or host-service\n"
       );
  return 200;
}

static void
find_host_or_service(const char* name, host** hst, service** svc)
{
  *svc = NULL;
  *hst = NULL;

  char* host_str    = strdup(name);
  char* service_str = strchr(host_str, '/');

  if (service_str) { /* host/service pair */
    *service_str++ = 0;
    *svc = find_service(host_str, service_str);
  } else { /* host */
    *hst = find_host(name);
  }

  free(host_str);
}

static int
toggle_notifications_for_obj(int sd, const char* obj, bool enable)
{
  host*    hst;
  service* svc;
  find_host_or_service(obj, &hst, &svc);

  if (svc) {
    (enable ? enable_service_notifications : disable_service_notifications)(svc);
    nsock_printf_nul(sd, "NOTIFICATIONS %sABLED FOR SERVICE: %s/%s\n", enable ? "EN" : "DIS", svc->host_name, svc->display_name);
    return 200;
  }

  if (hst) {
    (enable ? enable_host_notifications : disable_host_notifications)(hst);
    nsock_printf_nul(sd, "NOTIFICATIONS %sABLED FOR HOST: %s\n", enable ? "EN" : "DIS", hst->display_name);
    return 200;
  }

  nsock_printf_nul(sd, "NO HOST OR SERVICE FOUND FOR: %s", obj);
  return 404;
}

static int
schedule_downtime_for_obj(int sd, const char* obj, unsigned long minutes, char* comment_data)
{

  host*    hst;
  service* svc;
  find_host_or_service(obj, &hst, &svc);

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

static char*
format_service_state(int state)
{
  switch(state) {
  case STATE_OK:
    return "OK";
  case STATE_WARNING:
    return "WARNING";
  case STATE_UNKNOWN:
    return "UNKNOWN";
  case STATE_CRITICAL:
    return "CRITICAL";
  default:
    return NULL;
  }
}

static void
show_status_for_service(int sd, service* svc)
{
  int   state          = svc->current_state;
  char* output         = svc->plugin_output;
  char* friendly_state = format_service_state(state);

  if (friendly_state) {
    nsock_printf_nul(sd, "%s/%s;%s;%s\n",
      svc->host_name,
      svc->description,
      friendly_state,
      output);
  } else {
    nsock_printf_nul(sd, "Somehow Nagios thinks this state is something invalid: %i\n", state);
  }

  return;
}

static void
show_status_for_host(int sd, host* hst)
{
  servicesmember* svc_member = hst->services;

  while (svc_member) {
    show_status_for_service(sd, svc_member->service_ptr);
    svc_member = svc_member->next;
  }

  return;
}

static int
show_status_for_obj(int sd, const char* obj)
{
  host*    hst;
  service* svc;
  find_host_or_service(obj, &hst, &svc);

  if (svc) {
    show_status_for_service(sd, svc);
    return 200;
  }

  if (hst) {
    show_status_for_host(sd, hst);
    return 200;
  }

  nsock_printf_nul(sd, "NO HOST OR SERVICE FOUND FOR %s\n", obj);
  return 404;
}







static int
nez_cmd_help(int sd, char* object, char* rest)
{
  (void)object;
  (void)rest;
  return display_help(sd);
}

static int
nez_cmd_frenchman(int sd, char* object, char* rest)
{
  (void)object;
  (void)rest;
  nsock_printf_nul(sd, "yolochocinco!!!!!!\n");
  return 420;     // easter egg
}

static int
nez_cmd_status(int sd, char* object, char* rest)
{
  (void)rest;
  return show_status_for_obj(sd, object);
}

static int
nez_cmd_enable_notifications(int sd, char* object, char* rest)
{
  (void)rest;
  return toggle_notifications_for_obj(sd, object, true);
}

static int
nez_cmd_disable_notifications(int sd, char* object, char* rest)
{
  (void)rest;
  return toggle_notifications_for_obj(sd, object, false);
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

nez_command_t
nez_commands[] = {
  { "help", nez_cmd_help },
  { "yolo", nez_cmd_frenchman },
  { "status", nez_cmd_status },
  { "enable_notifications", nez_cmd_enable_notifications },
  { "disable_notifications", nez_cmd_disable_notifications },
  { "schedule_downtime", nez_cmd_schedule_downtime },
};
