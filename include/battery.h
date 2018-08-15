#ifndef _BATTERY_H
#define _BATTERY_H

#include "string-list.h"

#include <stdbool.h>

enum charging_status {
    UNKNOWN,
    DISCHARGING,
    CHARGING,
    FULL
};

struct bat_info {
    double percentage;
    double percentage_of_design;
    enum charging_status status;
};

bool is_battery(const char *name);
struct string_list *detect_batteries(void);
struct bat_info get_battery_info(struct string_list *batteries);

#endif
