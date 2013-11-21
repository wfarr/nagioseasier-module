#include <nagios/nagios.h>

#include "../helper.h"
#include "notifications.h"

static int
enable_notifications_for_obj(int sd, const char* obj, const char* rest)
{
  (void)rest;

  host*    hst;
  service* svc;
  nez_find_host_or_service(obj, &hst, &svc);

  if (svc) {
    enable_service_notifications(svc);
    nsock_printf_nul(sd, "NOTIFICATIONS ENABLED FOR SERVICE: %s/%s\n", svc->host_name, svc->display_name);
    return 200;
  }

  if (hst) {
    enable_host_notifications(hst);
    nsock_printf_nul(sd, "NOTIFICATIONS ENABLED FOR HOST: %s\n", hst->display_name);

    for(svc = hst->services; svc; svc = svc->services) {
      enable_notifications_for_obj(sd, hst->display_name + "/" + svc->description, NULL);
    }

    return 200;
  }

  nsock_printf_nul(sd, "NO HOST OR SERVICE FOUND FOR: %s", obj);
  return 404;
}

static int
disable_notifications_for_obj(int sd, const char* obj, const char* rest)
{
  (void)rest;

  host*    hst;
  service* svc;
  nez_find_host_or_service(obj, &hst, &svc);

  if (svc) {
    disable_service_notifications(svc);
    nsock_printf_nul(sd, "NOTIFICATIONS DISABLED FOR SERVICE: %s/%s\n", svc->host_name, svc->display_name);
    return 200;
  }

  if (hst) {
    disable_host_notifications(hst);
    nsock_printf_nul(sd, "NOTIFICATIONS DISABLED FOR HOST: %s\n", hst->display_name);

    for(svc = hst->services; svc; svc = svc->services) {
      disable_notifications_for_obj(sd, hst->display_name + "/" + svc->description, NULL);
    }

    return 200;
  }

  nsock_printf_nul(sd, "NO HOST OR SERVICE FOUND FOR: %s", obj);
  return 404;
}

int
nez_cmd_enable_notifications(int sd, char* object, char* rest)
{
  (void)rest;
  return enable_notifications_for_obj(sd, object, rest);
}

int
nez_cmd_disable_notifications(int sd, char* object, char* rest)
{
  (void)rest;
  return disable_notifications_for_obj(sd, object, rest);
}
