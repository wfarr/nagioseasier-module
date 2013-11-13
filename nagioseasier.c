#include <stdbool.h>

#include <nagios/nebmodules.h>
#include <nagios/nebcallbacks.h>

/*
  #include "../include/nebstructs.h"
  #include "../include/broker.h"
*/

#include <nagios/config.h>
#include <nagios/common.h>
#include <nagios/objects.h>
#include <nagios/nagios.h>

/* specify event broker API version (required) */
NEB_API_VERSION(CURRENT_NEB_API_VERSION);

static int nagioseasier_query_handler(int sd, char *buf, unsigned int len);
static int toggle_notifications_for_obj(const char *obj, bool enable);
static int string_equals(const char* a, const char* b);

/* this function gets called when the module is loaded by the event broker */
int nebmodule_init(int flags, char *args, nebmodule *handle)
{
  return qh_register_handler("nagioseasier", "The nagioseasier query handler", 0, nagioseasier_query_handler);
}

/* this function gets called when the module is unloaded by the event broker */
int nebmodule_deinit(int flags, int reason)
{
  return qh_deregister_handler("nagioseasier");
}

static int nagioseasier_query_handler(int sd, char *buf, unsigned int len)
{
  if (len == 0 || string_equals(buf, "help")) {
    nsock_printf_nul(sd, "Query handler for actually doing useful shit with this socket.\n"
		     "Available commands:\n"
		     "  enable_notifications    Enable notifications for a host or host-service\n"
		     "  disable_notifications   Disable notifications for a host or host-service\n"
		     );
    return 0;
  }

  const char *obj = "ops-breakin1-pe1-prd/last_puppet_run";
  char *space;

  if ((space = memchr(buf, ' ', len))) {
    *(space++) = 0;
  }

  if (!space && string_equals(buf, "enable_notifications")) {
    return toggle_notifications_for_obj(obj, true);
  }

  if (!space && string_equals(buf, "disable_notifications")) {
    return toggle_notifications_for_obj(obj, false);
  }

  nsock_printf_nul(sd, "UNKNOWN COMMAND");
  return 400;
}

static int toggle_notifications_for_obj(const char *obj, bool enable)
{
  host    *hst = find_host(obj);
  service *svc = find_service(obj);

  if (svc) {
    (enable ? enable_service_notifications : disable_service_notifications)(svc);
    nsock_printf_nul(sd, "NOTIFICATIONS %sABLED FOR SERVICE: %s", enable ? "EN" : "DIS", svc->display_name);
    return 200;
  }

  if (hst) {
    (enable ? enable_host_notifications : disable_host_notifications)(hst);
    nsock_printf_nul(sd, "NOTIFICATIONS %sABLED FOR HOST: %s", enable ? "EN" : "DIS", hst->display_name);
    return 200;
  }

  nsock_printf_nul(sd, "NO HOST OR SERVICE FOUND FOR: %s", obj);
  return 404;
}

static int string_equals(const char* a, const char* b)
{
  return strcmp(a, b) == 0;
}
