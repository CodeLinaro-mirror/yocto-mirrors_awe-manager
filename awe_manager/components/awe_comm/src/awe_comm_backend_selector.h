
#ifndef AWE_COMM_BACKEND_SELECTOR_H
#define AWE_COMM_BACKEND_SELECTOR_H
#include "awe_comm_backend.h"

awe_comm_backend* create_backend(awe_config *cfg_p, aweevt_listener cb, void* userdata);

#endif
