#include <stdlib.h>
#include "pilab-string.h"
#include "pilab-list.h"

/*
 * We need to create a chain! So we need an entry point.
 * Attach your pearls (nodes) to this!
 */

struct t_pilist *pilist_create()
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

struct t_pilist_node *pilist_create_node()
{
	struct t_pilist_node *new_node;

	new_node = malloc(sizeof(*new_node));

	if (!new_node)
		return NULL;

	new_node->next = NULL;
	new_node->data = NULL;

	return new_node;
}

/*
 * Free up the space allocated to a node.
 */

void pilist_free_node(struct t_pilist_node *node)
{
	if (!node)
		return;

	if (node->data)
		free(node->data);

	node->next = NULL;
	free(node);
	node = NULL;
}

/*
 * Free the complete list.
 */

void pilist_free(struct t_pilist *pilist)
{
	pilist_foreach_node(pilist, pilist_free_node);
	free(pilist);
	pilist = NULL;
}

/*
 * Stand up and rise again.
 */

static struct t_pilist_node *pilist_reuse_old_node(struct t_pilist *pilist)
{
	struct t_pilist_node *node_ptr;
	int position, i;

	if (!pilist || !pilist->head || pilist->capacity >= pilist->size)
		return NULL;

	position = pilist->capacity % pilist->size;

	node_ptr = pilist->head;
	for (i = 0; i < position; node_ptr = node_ptr->next, i++)
		;

	return node_ptr;
}

/*
 * Adds an element at the beginning of the list.
 *
 * This element is copied so it is safe to free the reference to data.
 */

void pilist_add(struct t_pilist *pilist, const void *data)
{
	struct t_pilist_node *new_node;

	if (!pilist || !data)
		return;

	new_node = pilist_reuse_old_node(pilist);

	if (new_node)
		; /* Good, we can reuse allocated memory */
	else if ((new_node = pilist_create_node()))
		pilist->capacity++;
	else
		return;

	pilist->size++;
	new_node->data = string_strdup((const char *)data);

	if (!pilist->head) {
		pilist->head = new_node;
		return;
	}

	new_node->next = pilist->head;
	pilist->head = new_node;
}

/*
 * Clear all the nodes in the list.
 *
 * This will reset the length of the list to 0, but maintains the capacity.
 */

void pilist_clear(struct t_pilist *pilist)
{
	struct t_pilist_node *node_ptr;

	if (!pilist || !pilist->head)
		return;

	for (node_ptr = pilist->head; node_ptr;
	     node_ptr = node_ptr->next, pilist->size--)
		if (node_ptr->data)
			free(node_ptr->data);
}

/*
 * Go over each node and call the given function.
 *
 * The function/callback takes a const void *.
 */

void pilist_foreach_node(struct t_pilist *pilist,
			 t_pilist_fmap_node *callback_fmap_node)
{
	struct t_pilist_node *node_ptr;

	if (!pilist || !callback_fmap_node || !pilist->head)
		return;

	for (node_ptr = pilist->head; node_ptr; node_ptr = node_ptr->next)
		callback_fmap_node(node_ptr);
}

/*
 * Go over each node's data and call the given function.
 *
 * The function/callback takes a struct t_pilist_node *.
 */

void pilist_foreach_node_data(struct t_pilist *pilist,
			      t_pilist_fmap_node_data *callback_fmap_node_data)
{
	struct t_pilist_node *node_ptr;

	if (!pilist || !callback_fmap_node_data || !pilist->head)
		return;

	for (node_ptr = pilist->head; node_ptr; node_ptr = node_ptr->next)
		callback_fmap_node_data(node_ptr->data);
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
 * Search the list for the node containing the data (case insensitive).
 *
 * Returns pointer to found data, NULL otherwise.
 */

struct t_pilist_node *pilist_casesearch(struct t_pilist *pilist,
					const void *data)
{
	struct t_pilist_node *node_ptr;

	if (!pilist || !pilist->head || !data)
		return NULL;

	for (node_ptr = pilist->head; node_ptr; node_ptr = node_ptr->next)
		if (string_strcasecmp((const char *)data,
				      (const char *)node_ptr->data) == 0)
			return node_ptr;

	/* data not found in the list */
	return NULL;
}

/*
 * Searches for an data in the list (case sensitive).
 *
 * Returns pointer to found data, NULL otherwise.
 */

struct t_pilist_node *pilist_search(struct t_pilist *pilist, const void *data)
{
	struct t_pilist_node *node_ptr;

	if (!pilist || !pilist->head || !data)
		return NULL;

	for (node_ptr = pilist->head; node_ptr; node_ptr = node_ptr->next)
		if (string_strcmp((const char *)data,
				  (const char *)node_ptr->data) == 0)
			return node_ptr;

	/* data not found in the list */
	return NULL;
}

/*
 * Search for an data in the list (case insensitive).
 *
 * Returns the position of the found data (>=0), -1 otherwise.
 */

int pilist_casesearch_pos(struct t_pilist *pilist, const void *data)
{
	struct t_pilist_node *node_ptr;
	int i;

	if (!pilist || !pilist->head || !data)
		return -1;

	i = 0;
	for (node_ptr = pilist->head; node_ptr; node_ptr = node_ptr->next, i++)
		if (string_strcasecmp((const char *)data,
				      (const char *)node_ptr->data) == 0)
			return i;

	/* data not found in the list */
	return -1;
}

/*
 * Search for an data in the list (case sensitive).
 *
 * Returns the position of the found data (>=0), -1 otherwise.
 */

int pilist_search_pos(struct t_pilist *pilist, const void *data)
{
	struct t_pilist_node *node_ptr;
	int i;

	if (!pilist || !pilist->head || !data)
		return -1;

	i = 0;
	for (node_ptr = pilist->head; node_ptr; node_ptr = node_ptr->next, i++)
		if (string_strcmp((const char *)data,
				  (const char *)node_ptr->data) == 0)
			return i;

	/* data not found in the list */
	return -1;
}

/*
 * Get a node from the list by position.
 *
 * Returns a node reference from the list, NULL otherwise.
 */

struct t_pilist_node *pilist_get_node(struct t_pilist *pilist, int position)
{
	struct t_pilist_node *node_ptr;
	int i;

	if (!pilist || !pilist->head || (position < 0) ||
	    (position > pilist->size))
		return NULL;

	i = 0;
	for (node_ptr = pilist->head; node_ptr; node_ptr = node_ptr->next, i++)
		if (i == position)
			return node_ptr;

	/* data not found in the list */
	return NULL;
}

/*
 * Get a node from the list by position.
 *
 * Returns a reference to the data, NULL otherwise.
 */

void *pilist_get_data(struct t_pilist *pilist, int position)
{
	struct t_pilist_node *node_ptr;
	node_ptr = pilist_get_node(pilist, position);
	return node_ptr ? node_ptr->data : NULL;
}

/*
 * Set a node's data field.
 */

void pilist_set_data(struct t_pilist_node *node, const void *data)
{
	if (!node || !data)
		return;

	if (node->data)
		free(node->data);
	node->data = string_strdup((const char *)data);
}

/*
 * Get string pointer to node data
 */

const char *pilist_node_data_string(struct t_pilist_node *node)
{
	if (node)
		return node->data;
	return NULL;
}

/*
 * Removes the node at a specific position.
 */

void pilist_remove_position(struct t_pilist *pilist, int position)
{
	struct t_pilist_node **node_ptr;
	struct t_pilist_node *trash_ptr;
	int i;

	if (!pilist || !pilist->head || (position < 0) ||
	    (position > pilist->size))
		return;

	node_ptr = &pilist->head;

	for (i = 0; i < position; i++)
		node_ptr = &(*node_ptr)->next;

	if (!*node_ptr)
		return;

	trash_ptr = *node_ptr;
	*node_ptr = trash_ptr->next;
	pilist_free_node(trash_ptr);
	pilist->size--;
	pilist->capacity--;
}

/*
 * Removes the first node with matching data.
 */

void pilist_remove_data(struct t_pilist *pilist, const void *data)
{
	struct t_pilist_node **node_ptr;
	struct t_pilist_node *trash_ptr;

	if (!pilist || !pilist->head || !data)
		return;

	node_ptr = &pilist->head;

	while ((*node_ptr)->data != data) {
		node_ptr = &(*node_ptr)->next;
	}

	if (!*node_ptr)
		return;

	trash_ptr = *node_ptr;
	*node_ptr = trash_ptr->next;
	pilist_free_node(trash_ptr);
	pilist->size--;
	pilist->capacity--;
}

/*
 * Remove a node from the list.
 */

void pilist_remove(struct t_pilist *pilist, struct t_pilist_node *node)
{
	struct t_pilist_node **node_ptr;
	struct t_pilist_node *trash_ptr;

	if (!pilist || !node || !pilist->head)
		return;

	node_ptr = &pilist->head;

	while (*node_ptr != node)
		node_ptr = &(*node_ptr)->next;

	if (!*node_ptr)
		return;

	trash_ptr = *node_ptr;
	*node_ptr = trash_ptr->next;
	pilist_free_node(trash_ptr);
	pilist->size--;
	pilist->capacity--;
}

/*
 * Removes all nodes from the list.
 *
 * NOTE: This function frees all the nodes, but not the list!
 */

void pilist_remove_all(struct t_pilist *pilist)
{
	struct t_pilist_node **node_ptr;
	struct t_pilist_node *trash_ptr;

	if (!pilist || !pilist->head)
		return;

	node_ptr = &pilist->head;

	while (*node_ptr) {
		trash_ptr = *node_ptr;
		node_ptr = &(*node_ptr)->next;
		pilist_remove(pilist, trash_ptr);
	}
}
