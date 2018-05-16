#ifndef _PILAB_ARRAYLIST_H
#define _PILAB_ARRAYLIST_H

typedef struct {
	int capacity;
	int length;
	void **items;
} array_list_t;


array_list_t* array_list_create(void);
void array_list_add(array_list_t* list, void* item);
void array_list_cat(array_list_t* source, array_list_t* target);
void array_list_foreach(array_list_t* list, void (*callback)(void* item));
void array_list_free(array_list_t* list);
void array_list_insert(array_list_t* list, void* item, int index);
void array_list_remove(array_list_t* list, int index);
void array_list_qsort(array_list_t* list, int compare(const void* a, const void* b));
int array_list_find(array_list_t* list, int compare(const void* a, const void* b), const void* data);

#endif
