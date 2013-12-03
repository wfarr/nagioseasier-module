#include <nagios/nagios.h>

#include "../helper.h"

#include "stats.h"


int nez_cmd_stats(int sd, char* object, char* rest)
{
  (void)object;
  (void)rest;

  int warning  = 0;
  int critical = 0;
  int unknown  = 0;
  int ok       = 0;

  char* json = "{ \"warning\": %d, \"unknown\": %d, \"critical\": %d, \"ok\": %d }\n";

  for (service* svc = service_list; svc; svc = svc->next) {
    switch(svc->current_state) {
    case STATE_WARNING:
      warning++;
      break;
    case STATE_UNKNOWN:
      unknown++;
      break;
    case STATE_CRITICAL:
      critical++;
      break;
    default:
      ok++;
      break;
    }
  }

  nsock_printf_nul(sd, json, warning, unknown, critical, ok);
  return 200;
}
