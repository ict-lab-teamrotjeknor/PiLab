#ifndef _PILAB_LIST_H
#define _PILAB_LIST_H

struct t_pilist_node;

typedef void(t_pilist_callback)(struct t_pilist_node *node);

struct t_pilist_node {
	/* hold any item */
	void *item;
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
extern void pilist_clear(struct t_pilist *pilist);
extern void pilist_free(struct t_pilist *pilist);
extern void pilist_add(struct t_pilist *pilist, void *item);
extern void pilist_foreach(struct t_pilist *pilist, t_pilist_callback callback);
extern int pilist_size(struct t_pilist *pilist);
extern int pilist_remove_item(struct t_pilist *pilist, void *item);
extern int pilist_remove_index(struct t_pilist *pilist, int index);

#endif
