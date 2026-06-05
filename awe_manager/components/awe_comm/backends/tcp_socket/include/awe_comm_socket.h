
#ifndef AWE_COMM_BACKEND_SOCKET_H
#define AWE_COMM_BACKEND_SOCKET_H

#include "awe_comm_backend.h"

awe_comm_backend* create_comm_backend_socket(awe_config *cfg_p, aweevt_listener cb, void* userdata);

#endif // AWE_COMM_BACKEND_SOCKET_H