#ifndef _PILAB_LIST_H
#define _PILAB_LIST_H

struct t_pilist_node;

typedef void(t_pilist_fmap_node)(struct t_pilist_node *node);
typedef void(t_pilist_fmap_node_data)(const void *item);

struct t_pilist_node {
	/* hold any data */
	void *data;
	/* singly linked */
	struct t_pilist_node *next;
};

struct t_pilist {
	/* internal head */
	struct t_pilist_node *head;
	/* size of the list */
	int size;
	/* capacity of the list */
	int capacity;
};

extern struct t_pilist *pilist_create(void);
extern struct t_pilist_node *pilist_create_node(void);
extern void pilist_free_node(struct t_pilist_node *node);
extern void pilist_free(struct t_pilist *pilist);
extern void pilist_add(struct t_pilist *pilist, const void *data);
extern void pilist_add_pointer(struct t_pilist *pilist, const void *data);
extern void pilist_add_last(struct t_pilist *pilist, const void *data);
extern void pilist_clear(struct t_pilist *pilist);
extern void pilist_foreach_node(struct t_pilist *pilist,
				t_pilist_fmap_node *callback_fmap_node);
extern void pilist_foreach_node_data(
	struct t_pilist *pilist,
	t_pilist_fmap_node_data *callback_fmap_node_data);
extern int pilist_size(struct t_pilist *pilist);
extern struct t_pilist_node *pilist_casesearch(struct t_pilist *pilist,
					       const void *data);
extern struct t_pilist_node *pilist_search(struct t_pilist *pilist,
					   const void *data);
extern int pilist_casesearch_pos(struct t_pilist *pilist, const void *data);
extern int pilist_search_pos(struct t_pilist *pilist, const void *data);
extern struct t_pilist_node *pilist_get_node(struct t_pilist *pilist,
					     int position);
extern void *pilist_get_data(struct t_pilist *pilist, int position);
extern void pilist_set_data(struct t_pilist_node *node, const void *data);
extern char *pilist_node_data_string(struct t_pilist_node *node);
extern void pilist_remove_position(struct t_pilist *pilist, int position);
extern void pilist_remove_data(struct t_pilist *pilist, void *data);
extern void pilist_remove(struct t_pilist *pilist, struct t_pilist_node *node);
extern void pilist_remove_all(struct t_pilist *pilist);

#endif
