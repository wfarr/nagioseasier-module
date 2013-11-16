#include <stdlib.h>
#include <string.h>

#include <nagios/common.h>
#include <nagios/nagios.h>
#include <nagios/objects.h>

#include "helper.h"

int
nez_string_equals(const char* a, const char* b)
{
  return strcmp(a, b) == 0;
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

void
nez_show_status_for_service(int sd, service* svc)
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
    nsock_printf_nul(sd, "SOMEHOW NAGIOS THINKS THIS STATE IS SOMETHING INVALID: %i\n", state);
  }

  return;
}

void
nez_show_status_for_host(int sd, host* hst)
{
  servicesmember* svc_member = hst->services;

  while (svc_member) {
    nez_show_status_for_service(sd, svc_member->service_ptr);
    svc_member = svc_member->next;
  }
  return;
}

void
nez_find_host_or_service(const char* name, host** hst, service** svc)
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
