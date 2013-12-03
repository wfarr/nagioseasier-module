#include <nagios/nagios.h>

#include "../helper.h"

#include "stats.h"

int
nez_cmd_stats(int sd, char* object, char* rest)
{
  // we take no arguments
  (void)object;
  (void)rest;

  // track object counts
  int ok       = 0;
  int warning  = 0;
  int critical = 0;
  int unknown  = 0;

  for (service* svc = service_list; svc; svc = svc->next) {
    switch(svc->current_state) {
    case STATE_OK:
      ok++;
      break;
    case STATE_WARNING:
      warning++;
      break;
    case STATE_CRITICAL:
      critical++;
      break;
    case STATE_UNKNOWN:
      unknown++;
      break;;
    default:
      break;
    }
  }

  nsock_printf_nul(sd,
    "ok: %d, warning: %d, critical: %d, unknown: %d",
    ok,
    warning,
    critical,
    unknown);

  return 200;
}