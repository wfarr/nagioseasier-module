#include <nagios/nagios.h>

#include "../helper.h"

static int
display_help(int sd)
{
  nsock_printf_nul(sd, "Query handler for actually doing useful shit with this socket.\n"
       "Available commands:\n"
       "  status <host|service>                          Display the status of a host or service\n"
       "  check <host|service>                           Schedule a re-check of the host or service\n"
       "\n"
       "  enable_notifications <host|service>            Enable notifications for a host or host-service\n"
       "  disable_notifications <host|service>           Disable notifications for a host or host-service\n"
       "\n"
       "  acknowledge <host|service> [<comment>]         Acknowledge a host/service problem (opt. comment)\n"
       "  unacknowledge <host|service>                   Unacknowledge a host/service problem\n"
       "\n"
       "  downtime <host|service> [<minutes> <comment>]  Schedule downtime for a host/service (opt. num minutes, comment)\n"
       "\n"
       "  problems [<svcgroup|hstgroup> <state>]         Display all services in a non-OK state\n"
       );
  return 200;
}

int
nez_cmd_help(int sd, char* object, char* rest)
{
  (void)object;
  (void)rest;
  return display_help(sd);
}
