#define _XOPEN_SOURCE 500

#include "string-list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct string_list *string_list_init(int size) {
	struct string_list *list = malloc(sizeof(*list) + size*sizeof(char *));
	list->size = size;
	list->length = 0;
	return list;
}

bool string_list_add(struct string_list *list, const char *string) {
	if (list->length == list->size) {
		fputs("List is somehow full\n", stderr);
		return false;
	}
	char *str_dup = strdup(string);
	if (str_dup == NULL) {
		fprintf(stderr, "Failed to duplicate string '%s' to add to list\n", string);
		return false;
	}
	list->list[list->length++] = str_dup;
	return true;
}

char *string_list_get(struct string_list *list, int index) {
	if (index >= list->length) {
		fputs("Somehow trying to index past end of list\n", stderr);
		return NULL;
	}
	return list->list[index];
}

bool string_list_contains(struct string_list *list, const char *string) {
	for (int i = 0; i < list->length; ++i) {
		if (strcmp(list->list[i], string) == 0) return true;
	}
	return false;
}

void string_list_destroy(struct string_list *list) {
	if (list == NULL) return;
	for (int i = 0; i < list->length; ++i) free(list->list[i]);
	free(list);
}
