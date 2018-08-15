#define _POSIX_C_SOURCE 200809L

#include "battery.h"
#include "string-list.h"

#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool is_battery(const char *name) {
	char path[64];
	snprintf(path, 64, "/sys/class/power_supply/%s/type", name);
	FILE *stream = fopen(path, "r");
	if (stream == NULL) {
		fprintf(stderr, "Failed to open battery type file for '%s'\n", name);
		return false;
	}

	char buf[8]; // "Battery\n"
	bool is_battery = fread(buf, 1, sizeof(buf), stream) == 8 &&
		getc(stream) == EOF && strncmp(buf, "Battery\n", 8) == 0;

	fclose(stream);
	return is_battery;
}

struct string_list *detect_batteries(void) {
	DIR *parent_dir = opendir("/sys/class/power_supply");
	if (parent_dir == NULL) {
		fputs("Failed to open directory of power supplies\n", stderr);
		return NULL;
	}

	struct dirent *dir;
	int number_of_batteries = 0;
	while ((dir = readdir(parent_dir)) != NULL) ++number_of_batteries;

	rewinddir(parent_dir);
	struct string_list *batteries = string_list_init(number_of_batteries);
	while ((dir = readdir(parent_dir)) != NULL) {
		if (dir->d_name[0] != '.' && is_battery(dir->d_name)) {
			string_list_add(batteries, dir->d_name);
		}
	}
	closedir(parent_dir);
	return batteries;
}

struct bat_info get_battery_info(struct string_list *batteries) {
	struct bat_info info = {0};

	int remaining = 0;
	int full = 0;
	int full_design = 0;

	for (int i = 0; i < batteries->length; ++i) {
		char *name = string_list_get(batteries, i);

		char path[64];
		snprintf(path, 64, "/sys/class/power_supply/%s/uevent", name);
		FILE *stream = fopen(path, "r");
		if (stream == NULL) {
			fprintf(stderr, "Failed to open battery file for '%s'\n", name);
			break;
		}

		char *line = NULL;
		size_t len = 0;
		ssize_t nread;
		while ((nread = getline(&line, &len, stream)) != -1) {
			if (strncmp(line, "POWER_SUPPLY_STATUS=", 20) == 0) {
				switch (line[20]) { // just assign one
					case 'C': info.status = CHARGING; break;
					case 'D': info.status = DISCHARGING; break;
					case 'F': info.status = FULL; break;
				} // defaults to UNKNOWN, so ignore anything else
			} else if (strncmp(line, "POWER_SUPPLY_ENERGY_NOW=", 24) == 0) {
				remaining += atoi(line + 24);
			} else if (strncmp(line, "POWER_SUPPLY_ENERGY_FULL=", 25) == 0) {
				full += atoi(line + 25);
			} else if (strncmp(line, "POWER_SUPPLY_ENERGY_FULL_DESIGN=", 32) == 0) {
				full_design += atoi(line + 32);
			}
		}
		fclose(stream);
		free(line);
	}

	info.percentage = (double) remaining/full;
	info.percentage_of_design = (double) remaining/full_design;
	return info;
}
