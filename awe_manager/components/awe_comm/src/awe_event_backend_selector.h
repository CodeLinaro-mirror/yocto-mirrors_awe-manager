
#ifndef AWE_EVENT_BACKEND_SELECTOR_H
#define AWE_EVENT_BACKEND_SELECTOR_H
#include "awe_event_backend.h"

awe_evt_backend* create_event_backend(awe_config *cfg_p, aweevt_listener cb, void* userdata);

#endif
