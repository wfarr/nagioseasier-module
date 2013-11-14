#ifndef HELPER_H
#define HELPER_H

#define countof(x) (sizeof(x) / sizeof(*(x)))

int nez_string_equals(const char* a, const char* b);

void nez_find_host_or_service(const char* name, host** hst, service** svc);

void nez_show_status_for_service(int sd, service* svc);
void nez_show_status_for_host(int sd, host* hst);


#endif
