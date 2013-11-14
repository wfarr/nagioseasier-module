#include <nagios/nagios.h>

#include "../helpers.h"
#include "check.h"

static int
force_nagios_check(int sd, char* obj)
{
  host*    hst;
  service* svc;
  nez_find_host_or_service(obj, &hst, &svc);

  time_t delay_time = time(NULL) + 30L;

  if (svc) {
    schedule_service_check(svc, delay_time, CHECK_OPTION_FORCE_EXECUTION);
    nsock_printf_nul(sd, "SCHEDULED CHECK FOR SERVICE %s\n", obj);
    return 200;
  }

  if (hst) {
    schedule_host_check(hst, delay_time, CHECK_OPTION_FORCE_EXECUTION);

    servicesmember* services = hst->services;
    for (; services; services = services->next) {
      service* svc = services->service_ptr;

      // schedule the service checks to occur 30s after the host check
      schedule_service_check(svc, delay_time + 30L, CHECK_OPTION_FORCE_EXECUTION);
    }

    nsock_printf_nul(sd, "SCHEDULED CHECKS FOR HOST %s AND ALL ITS SERVICES\n", obj);
    return 200;
  }

  nsock_printf_nul(sd, "COULD NOT FIND HOST OR SERVICE %s\n", obj);
  return 404;
}

int
nez_cmd_check(int sd, char* obj, char* rest)
{
  (void)rest;
  return force_nagios_check(sd, obj);
}
