#include "arraylist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

array_list_t* create_array_list(void)
{
	array_list_t* list = (array_list_t*)malloc(sizeof(array_list_t));

	if (!list)
		return NULL;

	list->capacity = 10;
	list->length = 0;
	list->items = malloc(sizeof(void *) * list->capacity);
	return list;
}

void array_list_resize(array_list_t* list)
{
	if (!list)
		return;

	if (list->length == list->capacity) {
		list->capacity += 10;
		list->items = realloc(list->items, sizeof(void*) *list->capacity);
	}
}

void array_list_add(array_list_t* list, void* item)
{
	if (!list)
		return;

	array_list_resize(list);
	list->items[list->length++] = item;

}

void array_list_insert(array_list_t* list, void* item, int index)
{
	if (!list)
		return;

	array_list_resize(list);
	memmove(&list->items[index + 1],
			&list->items[index],
			sizeof(void*) * (list->length - index));
	list->items[index] = item;
	list->length++;
}
void array_list_remove(array_list_t* list, int index)
{
	if (!list)
		return;

	list->length--;
	memmove(&list->items[index],
			&list->items[index +1],
			sizeof(void*) * (list->length - index));
}

void array_list_free(array_list_t* list)
{
	if (!list)
		return;

	free(list->items);
	free(list);
}

void array_list_cat(array_list_t* dest, array_list_t* src)
{
	if (!src || !dest)
		return;

	for (int i = 0; i < src->length; ++i) {
		array_list_add(dest, src->items[i]);
	}
}

void array_list_foreach(array_list_t* list, void (*callback)(void* item))
{
	if (!list || !callback)
		return;

	for (int i = 0; i < list->length; ++i) {
		callback(list->items[i]);
	}
}

void array_list_qsort(array_list_t* list, int compare(const void* a, const void* b))
{
	if (!list || !compare) 
		return;

	qsort(list->items, list->length, sizeof(void*), compare);
}
