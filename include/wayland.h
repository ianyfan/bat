#ifndef _WAYLAND_H
#define _WAYLAND_H

#include "bat.h"

#include <stdbool.h>

bool init_wayland(struct bat_state *state);
void finish_wayland(struct bat_state *state);

#endif
