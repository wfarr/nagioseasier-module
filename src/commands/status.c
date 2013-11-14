#include <nagios/nagios.h>

#include "../helper.h"

#include "status.h"

static int
show_status_for_obj(int sd, const char* obj)
{
  host*    hst;
  service* svc;
  nez_find_host_or_service(obj, &hst, &svc);

  if (svc) {
    nez_show_status_for_service(sd, svc);
    return 200;
  }

  if (hst) {
    nez_show_status_for_host(sd, hst);
    return 200;
  }

  nsock_printf_nul(sd, "NO HOST OR SERVICE FOUND FOR %s\n", obj);
  return 404;
}

int
nez_cmd_status(int sd, char* object, char* rest)
{
  (void)rest;

  if (object) {
    return show_status_for_obj(sd, object);
  }

  nsock_printf_nul(sd, "#nagioseasier status requires one argument!\n");
  return 400;
}
