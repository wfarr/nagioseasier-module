/* system shit */
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* nagios shit */
#include <nagios/common.h>
#include <nagios/config.h>
#include <nagios/downtime.h>
#include <nagios/objects.h>
#include <nagios/nagios.h>
#include <nagios/nebmodules.h>
#include <nagios/nebcallbacks.h>

/*
  #include "../include/nebstructs.h"
  #include "../include/broker.h"
*/

/* specify event broker API version (required) */
NEB_API_VERSION(CURRENT_NEB_API_VERSION)

/* copied to stop the compiler bitching */
int qh_deregister_handler(const char* name);
int qh_register_handler(const char* name, const char* description, unsigned int options, qh_handler handler);

static int nagioseasier_query_handler(int sd, char* buf, unsigned int len);
static int display_help(int sd);
static int toggle_notifications_for_obj(int sd, const char* obj, bool enable);
static int schedule_downtime_for_obj(int sd, const char* obj, unsigned long int minutes);
static int show_status_for_obj(int sd, const char* obj);
static int string_equals(const char* a, const char* b);

/* this function gets called when the module is loaded by the event broker */
int
nebmodule_init(int flags, char* args, nebmodule* handle)
{
  (void)flags;
  (void)args;
  (void)handle;

  return qh_register_handler("nagioseasier", "The nagioseasier query handler", 0, nagioseasier_query_handler);
}

/* this function gets called when the module is unloaded by the event broker */
int
nebmodule_deinit(int flags, int reason)
{
  (void)flags;
  (void)reason;

  return qh_deregister_handler("nagioseasier");
}

static int
nagioseasier_query_handler(int sd, char* buf, unsigned int len)
{
  if (len == 0) {
    return display_help(sd);
  }

  char* action = buf;
  char* obj;
  char* rest;

  /* separate our action and obj */
  if ((obj = memchr(buf, ' ', len))) {
    *(obj++) = 0;
  }

  /* shove the rest of the input into rest, leave obj alone */
  if ((rest = memchr(obj, ' ', strlen(obj)))) {
    *(rest++) = 0;
  }

  if (!obj && string_equals(action, "help")) {
    return display_help(sd);
  }

  if (!obj && string_equals(action, "yolo")) {
    nsock_printf_nul(sd, "yolochocinco!!!!!!\n");
    return 420;     // easter egg
  }

  if (obj && (string_equals(action, "status"))) {
    return show_status_for_obj(sd, obj);
  }

  if (obj && (string_equals(action, "enable_notifications") || string_equals(action, "unmute"))) {
    return toggle_notifications_for_obj(sd, obj, true);
  }

  if (obj && (string_equals(action, "disable_notifications") || string_equals(action, "mute"))) {
    return toggle_notifications_for_obj(sd, obj, false);
  }

  if (obj && (string_equals(action, "schedule_downtime"))) {
    unsigned long int minutes = 15L;

    // assume the next argument is number of minutes
    if (rest) {
      char *garbage;

      if ((garbage = memchr(rest, ' ', strlen(rest)))) {
        *(garbage++) = 0;
      }

      minutes = strtoul(rest, NULL, 10);

      if (minutes <= 0) {
	minutes = 15L;
      }
    }

    return schedule_downtime_for_obj(sd, obj, minutes);
  }

  nsock_printf_nul(sd, "UNKNOWN COMMAND\n");
  return 400;
}


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
schedule_downtime_for_obj(int sd, const char* obj, unsigned long int minutes)
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
    char*         comment_data = "lol this is a test";
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

/* static int */
/* acknowledge_obj_problem(int sd, const char *obj) */
/* { */
/* // TODO */
/* } */

/* static int */
/* unacknowledge_obj_problem(int sd, const char* obj) */
/* { */
/* // TODO */
/* } */

/* static int */
/* schedule_downtime_for_obj(int sd, const char *obj, int minutes) */
/* { */
/* // TODO */
/* } */

/* static int */
/* show_problems(int sd, const char *obj) */
/* { */
/* // TODO */
/* } */

/* static int */
/* show_muted(int sd) */
/* { */
/* // TODO */
/* } */

static int string_equals(const char* a, const char* b)
{
  return strcmp(a, b) == 0;
}
