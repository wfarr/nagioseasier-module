#include <string.h>

#include <nagios/downtime.h>
#include <nagios/nagios.h>

#include "helper.h"
#include "commands.h"
#include "commands/check.h"
#include "commands/help.h"
#include "commands/notifications.h"
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

static int
find_service_state(char* state) {
  if (nez_string_equals(state, "ok") || nez_string_equals(state, "OK")) {
    return STATE_OK;
  }

  if (nez_string_equals(state, "warning") || nez_string_equals(state, "WARNING")) {
    return STATE_WARNING;
  }

  if (nez_string_equals(state, "unknown") || nez_string_equals(state, "UNKNOWN")) {
    return STATE_UNKNOWN;
  }

  if (nez_string_equals(state, "critical") || nez_string_equals(state, "CRITICAL")) {
    return STATE_CRITICAL;
  }

  return 404;
}

static int
acknowledge_problem_for_obj(int sd, const char* obj, char* comment)
{
  host*    hst;
  service* svc;
  nez_find_host_or_service(obj, &hst, &svc);

  char* author     = "nagioseasier";
  int   sticky     = 1; // DO NOT send notifications until this service/host recovers
  int   notify     = 1; // DO send a notification that we have ack'd this
  int   persistent = 0; // we don't want persistent comments in Nagios

  if (svc) {
    acknowledge_service_problem(
      svc,
      author,
      comment,
      sticky,
      notify,
      persistent);

    nsock_printf_nul(sd, "ACKNOWLEDGED PROBLEMS ON %s WITH: %s\n", obj, comment);
    return 200;
  }

  if (hst) {
    acknowledge_host_problem(
      hst,
      author,
      comment,
      sticky,
      notify,
      persistent);

    nsock_printf_nul(sd, "ACKNOWLEDGED PROBLEMS ON %s WITH: %s\n", obj, comment);
    return 200;
  }

  nsock_printf_nul(sd, "NO HOST OR SERVICE FOUND FOR %s\n", obj);
  return 404;
}

static int
unacknowledge_problem_for_obj(int sd, const char* obj)
{
  host*    hst;
  service* svc;
  nez_find_host_or_service(obj, &hst, &svc);

  if (svc) {
    remove_service_acknowledgement(svc);
    nsock_printf_nul(sd, "REMOVED ACKNOWLEDGEMENT ON %s\n", obj);
    return 200;
  }

  if (hst) {
    remove_host_acknowledgement(hst);
    nsock_printf_nul(sd, "REMOVED ACKNOWLEDGEMENT ON %s\n", obj);
    return 200;
  }

  nsock_printf_nul(sd, "NO HOST OR SERVICE FOUND FOR %s\n", obj);
  return 404;
}

static void
show_status_for_service_by_state(int sd, int state, service* svc)
{
  if (state == 404 && svc->current_state != STATE_OK) {
    nez_show_status_for_service(sd, svc);
    return;
  }

  if (state != 404 && svc->current_state == state) {
    nez_show_status_for_service(sd, svc);
    return;
  }
}

static void
filter_servicesmember_by_state(int sd, int state, servicesmember* services)
{
  for (; services; services = services->next) {
    show_status_for_service_by_state(sd, state, services->service_ptr);
  }
}

static void
filter_services_by_state(int sd, int state, service* svc)
{
  for (; svc; svc = svc->next) {
    show_status_for_service_by_state(sd, state, svc);
  }
}

static int
display_service_problems(int sd, char* str, char* state)
{
  // set the desired service state
  int state_filter = 404;
  if (state) {
    state_filter = find_service_state(state);
  }

  if (str) {
    // walk servicegroups and look for a match
    for (servicegroup* svc_group = servicegroup_list; svc_group; svc_group = svc_group->next) {
      if (nez_string_equals(str, svc_group->group_name)) {
        filter_servicesmember_by_state(sd, state_filter, svc_group->members);
        return 200;
      }
    }

    // walk hostgroups and look for a match
    for (hostgroup* hst_group = hostgroup_list; hst_group; hst_group = hst_group->next) {
      if (nez_string_equals(str, hst_group->group_name)) {
        for (hostsmember* h = hst_group->members; h; h = h->next) {
          filter_servicesmember_by_state(sd, state_filter, h->host_ptr->services);
          return 200;
        }
      }
    }

    nsock_printf_nul(sd, "COULD NOT FIND SERVICEGROUP OR HOSTGROUP %s\n", str);
    return 404;
  }

  filter_services_by_state(sd, state_filter, service_list);
  return 200;
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
nez_cmd_acknowledge(int sd, char* object, char* rest)
{
  return acknowledge_problem_for_obj(sd, object, rest);
}

static int
nez_cmd_unacknowledge(int sd, char* object, char* rest)
{
  (void)rest;
  return unacknowledge_problem_for_obj(sd, object);
}

static int
nez_cmd_problems(int sd, char* object, char* rest)
{
  return display_service_problems(sd, object, rest);
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
