#include "pilab-list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * We need to create a chain! So we need an entry point.
 * Attach your pearls (nodes) to this!
 */

struct t_pilist *pilist_create(void)
{
	struct t_pilist *new_list;
	new_list = malloc(sizeof(*new_list));
	if (!new_list)
		return NULL;
	new_list->size = new_list->capacity = 0;
	new_list->head = NULL;
	return new_list;
}

/*
 * Conjure up a new node.
 */

static struct t_pilist_node *pilist_create_node(void)
{
	struct t_pilist_node *new_node;
	new_node = malloc(sizeof(*new_node));
	if (!new_node)
		return NULL;
	new_node->next = NULL;
	new_node->item = NULL;
	return new_node;
}

/*
 * Free up the slot in the chain.
 */

static void pilist_free_node(struct t_pilist_node *node)
{
	/* no need to check arg, if null free wil abort */
	free(node);
	node = NULL;
}

/*
 * Clears the whole list but keeps the t_pilist reference
 */

void pilist_clear(struct t_pilist *pilist)
{
	if (!pilist || !pilist->head)
		return;
	struct t_pilist_node **current;
	/* struct t_pilist_node** delete; */
	current = &pilist->head;
	while (*current) {
		/* delete = current; */
		current = &(*current)->next;
		/* pilist_free_node(*delete); */
	}
	pilist->size = 0;
}

/*
 * Free the complete list.
 */

void pilist_free(struct t_pilist *pilist)
{
	pilist_foreach(pilist, pilist_free_node);
	free(pilist);
	pilist = NULL;
}

/*
 * Stand up and rise again.
 */

static struct t_pilist_node *pilist_reuse_old_node(struct t_pilist *pilist)
{
	if (!pilist || !pilist->head || pilist->capacity >= pilist->size)
		return NULL;
	struct t_pilist_node **current;
	int index = pilist->capacity % pilist->size;
	current = &pilist->head;
	for (int i = 0; i < index; ++i)
		current = &(*current)->next;
	return *current;
}

/*
 * Adds an element at the beginning of the list.
 */

void pilist_add(struct t_pilist *pilist, void *item)
{
	if (!pilist || !item)
		return;

	struct t_pilist_node *new_node;
	new_node = pilist_reuse_old_node(pilist);
	if (new_node)
		; /* Good we can reuse allocated memory */
	else if ((new_node = pilist_create_node()))
		pilist->capacity++;
	else
		return;
	pilist->size++;
	new_node->item = item;
	if (!pilist->head) {
		pilist->head = new_node;
		return;
	}
	new_node->next = pilist->head;
	pilist->head = new_node;
}

/*
 * Go over each node and calls the given function.
 *
 * The function takes a struct t_pilist_node.
 */

void pilist_foreach(struct t_pilist *pilist, t_pilist_callback callback)
{
	if (!pilist || !callback || !pilist->head)
		return;
	struct t_pilist_node **current;
	current = &pilist->head;
	while (*current) {
		callback(*current);
		*current = (*current)->next;
	}
}

/*
 * Returns the size of the list, -1 for null reference.
 */

int pilist_size(struct t_pilist *pilist)
{
	if (!pilist)
		return -1;
	return pilist->size;
}

/*
 * Removes the node at a specific index.
 *
 * Returns the index of the removed node, -1 otherwise.
 */

int pilist_remove_index(struct t_pilist *pilist, int index)
{
	if (!pilist || (index < 0) || (index >= pilist->size))
		return -1;
	struct t_pilist_node **current;
	struct t_pilist_node *trash;
	current = &pilist->head;
	for (int i = 0; i < index; i++)
		current = &(*current)->next;
	trash = *current;
	*current = trash->next;
	pilist_free_node(trash);
	pilist->size--;
	return index;
}

/*
 * Removes a node matching the given item.
 *
 * Returns the index of the removed node, -1 otherwise.
 */

int pilist_remove_item(struct t_pilist *pilist, void *item)
{
	if (!pilist || !item)
		return -1;
	struct t_pilist_node **current;
	struct t_pilist_node *trash;
	current = &pilist->head;
	int index = 0;
	while (*current && (*current)->item != item) {
		current = &(*current)->next;
		index++;
	}
	if (!*current)
		return -1;
	trash = *current;
	*current = trash->next;
	pilist_free_node(trash);
	pilist->size--;
	return index;
}
