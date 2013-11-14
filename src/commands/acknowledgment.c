#include <nagios/nagios.h>

#include "../helpers.h"
#include "acknowledgment.h"

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

int
nez_cmd_acknowledge(int sd, char* object, char* rest)
{
  return acknowledge_problem_for_obj(sd, object, rest);
}

int
nez_cmd_unacknowledge(int sd, char* object, char* rest)
{
  (void)rest;
  return unacknowledge_problem_for_obj(sd, object);
}
