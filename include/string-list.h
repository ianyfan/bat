#ifndef _STRING_LIST_H
#define _STRING_LIST_H

/* since this list is only used internally and its use cases known exactly,
 * it makes many assumptions about its use in order to minimise implementation,
 * namely that its maximum length is known beforehand
 * and that items will never be removed from it;
 * index checks are merely for sanity, since it should never out-of-bounds
 */

#include <stdbool.h>

struct string_list {
	int length;
	int size;
	char *list[];
};

struct string_list *string_list_init(int size);
bool string_list_add(struct string_list *list, const char *string);
char *string_list_get(struct string_list *list, int index);
bool string_list_contains(struct string_list *list, const char *string);
void string_list_destroy(struct string_list *list);

#endif
