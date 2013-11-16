#include <nagios/nagios.h>

#include "../helper.h"
#include "problems.h"

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

    for (host* hst = host_list; hst; hst = hst->next) {
      if (nez_string_equals(str, hst->name)) {
        filter_servicesmember_by_state(sd, state_filter, h->services);
        return 200;
      }
    }

    nsock_printf_nul(sd, "COULD NOT FIND SERVICEGROUP OR HOSTGROUP %s\n", str);
    return 404;
  }

  filter_services_by_state(sd, state_filter, service_list);
  return 200;
}

int
nez_cmd_problems(int sd, char* object, char* rest)
{
  return display_service_problems(sd, object, rest);
}
