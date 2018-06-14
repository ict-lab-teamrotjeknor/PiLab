#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pilab-string.h"
#include "pilab-hashtable.h"
#include "pilab-list.h"

char *hashtable_type_string[HASHTABLE_NUM_TYPES] = { PILAB_HASHTABLE_INTEGER,
						     PILAB_HASHTABLE_STRING,
						     PILAB_HASHTABLE_POINTER };

/*
 * Search for a hashtable type.
 *
 * Return index of type in enum t_hashtable_type, -1 if the type could not be
 * found.
 */

int hashtable_get_type(const char *type)
{
	if (!type)
		return -1;

	for (int i = 0; i < HASHTABLE_NUM_TYPES; ++i)
		if (string_charcasecmp(hashtable_type_string[i], type) == 0)
			return i;

	/* type was not found */
	return -1;
}

/*
 * Hashes a string using a variant of djb2 hash.
 *
 * Return the hash of the string.
 */

unsigned long long hashtable_hash_key_djb2(const char *string)
{
	unsigned long long hash;
	const char *ptr_string;

	hash = 5381;
	for (ptr_string = string; ptr_string[0]; ptr_string++)
		hash ^= (hash << 5) + (hash >> 2) + (int)(ptr_string[0]);
	return hash;
}

/*
 * Hashes a key (default callback).
 *
 * Return the hash of the key, depending on the type of the key.
 */

unsigned long long hashtable_hash_key_default_cb(struct t_hashtable *hashtable,
						 const void *key)
{
	unsigned long long hash;

	hash = 0;

	switch (hashtable->type_keys) {
	case HASHTABLE_INTEGER:
		hash = (unsigned long long)(*((int *)key));
		break;
	case HASHTABLE_STRING:
		hash = hashtable_hash_key_djb2((const char *)key);
		break;
	case HASHTABLE_POINTER:
		hash = (unsigned long long)((unsigned long)((void *)key));
		break;
	case HASHTABLE_NUM_TYPES:
		break;
	}

	return hash;
}

/*
 * Compare two entry keys (default callback).
 *
 * Returns:
 * -1: key1 < key2
 *  0: key1 == key2
 *  1: key1 > key2
 */

int hashtable_keycmp_default_cb(struct t_hashtable *hashtable, const void *key1,
				const void *key2)
{
	int rc = 0;

	switch (hashtable->type_keys) {
	case HASHTABLE_INTEGER:
		if (*((int *)key1) < *((int *)key2))
			rc = -1;
		else if (*((int *)key1) > *((int *)key2))
			rc = 1;
		break;
	case HASHTABLE_STRING:
		rc = string_strcmp((const char *)key1, (const char *)key2);
		break;
	case HASHTABLE_POINTER:
		if (key1 < key2)
			rc = -1;
		else if (key1 > key2)
			rc = 1;
		break;
	case HASHTABLE_NUM_TYPES:
		break;
	}

	return rc;
}

/*
 * Conjure up a new hashtable.
 *
 * THE SIZE IS THE VALUE OF THE INTERNAL ARRAY, USED TO STORE THE HASHED KEYS!
 *
 * Returns pointer to a new hashtable, NULL otherwise.
 */

struct t_hashtable *hashtable_create(int size, const char *type_keys,
				     const char *type_values,
				     t_hashtable_hash_key *callback_hash_key,
				     t_hashtable_keycmp *callback_keycmp)
{
	struct t_hashtable *new_hashtable;
	int type_keys_int, type_values_int;

	if (size <= 0)
		return NULL;

	type_keys_int = hashtable_get_type(type_keys);
	if (type_keys_int < 0)
		return NULL;

	type_values_int = hashtable_get_type(type_values);
	if (type_values_int < 0)
		return NULL;

	new_hashtable = malloc(sizeof(*new_hashtable));
	if (!new_hashtable)
		return NULL;

	if (new_hashtable) {
		new_hashtable->size = size;
		new_hashtable->type_keys = type_keys_int;
		new_hashtable->type_values = type_values_int;
		new_hashtable->htable =
			calloc(size, sizeof(*(new_hashtable->htable)));
		new_hashtable->keys_values = NULL;
		if (!new_hashtable->htable) {
			free(new_hashtable);
			return NULL;
		}
		new_hashtable->count = 0;
		if (callback_hash_key)
			new_hashtable->callback_hash_key = callback_hash_key;
		else
			new_hashtable->callback_hash_key =
				&hashtable_hash_key_default_cb;
		if (callback_keycmp)
			new_hashtable->callback_keycmp = callback_keycmp;
		else
			new_hashtable->callback_keycmp =
				&hashtable_keycmp_default_cb;
		new_hashtable->callback_free_key = NULL;
		new_hashtable->callback_free_value = NULL;
	}

	return new_hashtable;
}

/*
 * Allocate space for a key or value.
 */

void hashtable_alloc_type(enum t_hashtable_type type, const void *value,
			  void **pointer)
{
	switch (type) {
	case HASHTABLE_INTEGER:
		if (value) {
			*pointer = malloc(sizeof(int));
			if (*pointer)
				*((int *)(*pointer)) = *((int *)value);
		} else {
			*pointer = NULL;
		}
		break;
	case HASHTABLE_STRING:
		*pointer = (value) ? string_strdup((const char *)value) : NULL;
		break;
	case HASHTABLE_POINTER:
		*pointer = (void *)value;
		break;
	case HASHTABLE_NUM_TYPES:
		break;
	}
}

/*
 * Free the space used by a key.
 */

void hashtable_free_key(struct t_hashtable *hashtable,
			struct t_hashtable_item *item)
{
	if (!hashtable)
		return;

	if (hashtable->callback_free_key) {
		(void)(hashtable->callback_free_key)(hashtable, item->key);
	} else {
		switch (hashtable->type_keys) {
		case HASHTABLE_INTEGER:
		case HASHTABLE_STRING:
		case HASHTABLE_POINTER:
			break;
		case HASHTABLE_NUM_TYPES:
			break;
		}
	}
}

/*
 * Free the space used by a value.
 */

void hashtable_free_value(struct t_hashtable *hashtable,
			  struct t_hashtable_item *item)
{
	if (!hashtable)
		return;

	if (hashtable->callback_free_value) {
		(void)(hashtable->callback_free_value)(hashtable, item->key,
						       item->value);
	} else {
		switch (hashtable->type_keys) {
		case HASHTABLE_INTEGER:
		case HASHTABLE_STRING:
		case HASHTABLE_POINTER:
			break;
		case HASHTABLE_NUM_TYPES:
			break;
		}
	}
}

/*
 * Sets value for a key in hashtable.
 *
 * Returns pointer to item created/updated, NULL if error.
 */

struct t_hashtable_item *hashtable_set(struct t_hashtable *hashtable,
				       const void *key, const void *value)
{
	unsigned long long hash;
	struct t_hashtable_item *item_ptr, *item_pos, *new_item;

	if (!hashtable || !key)
		return NULL;

	/* position of item in hashtable */
	hash = hashtable->callback_hash_key(hashtable, key) % hashtable->size;
	item_pos = NULL;
	for (item_ptr = hashtable->htable[hash];
	     item_ptr && ((int)(hashtable->callback_keycmp)(hashtable, key,
							    item_ptr->key) > 0);
	     item_ptr = item_ptr->next) {
		item_pos = item_ptr;
	}

	/* replace value if item is already in hashtable */
	if (item_ptr &&
	    (hashtable->callback_keycmp(hashtable, key, item_ptr->key) == 0)) {
		hashtable_free_value(hashtable, item_ptr);
		hashtable_alloc_type(hashtable->type_values, value,
				     &item_ptr->value);
	}

	/* create the new item */
	new_item = malloc(sizeof(*new_item));
	if (!new_item)
		return NULL;

	/* set key and value */
	hashtable_alloc_type(hashtable->type_keys, key, &new_item->key);
	hashtable_alloc_type(hashtable->type_values, value, &new_item->value);

	/* add the item */
	if (item_pos) {
		/* insert item after position is found */
		new_item->prev = item_pos;
		new_item->next = item_pos->next;
		if (item_pos->next)
			(item_pos->next)->prev = new_item;
		item_pos->next = new_item;
	} else {
		/* first in the list, safe to inject */
		new_item->prev = NULL;
		new_item->next = hashtable->htable[hash];
		if (hashtable->htable[hash])
			(hashtable->htable[hash])->prev = new_item;
		hashtable->htable[hash] = new_item;
	}

	hashtable->count++;
	return new_item;
}

/*
 * Search the hashtable for an item.
 *
 * When the hash is non NULL, it will use the provided hash to try to get the
 * item (even if the key could not be found).
 *
 * Returns pointer to t_hashtable_item, if key found, NULL otherwise.
 */

struct t_hashtable_item *hashtable_get_item(struct t_hashtable *hashtable,
					    const void *key,
					    unsigned long long *hash)
{
	unsigned long long key_hash;
	struct t_hashtable_item *item_ptr;

	if (!hashtable || !key)
		return NULL;

	key_hash =
		hashtable->callback_hash_key(hashtable, key) % hashtable->size;
	if (hash)
		*hash = key_hash;

	for (item_ptr = hashtable->htable[key_hash];
	     item_ptr &&
	     hashtable->callback_keycmp(hashtable, key, item_ptr->key) > 0;
	     item_ptr = item_ptr->next)
		;

	if (item_ptr &&
	    hashtable->callback_keycmp(hashtable, key, item_ptr->key) == 0)
		return item_ptr;

	return NULL;
}

/*
 * Get the value for a key in the hastable.
 *
 * Returns pointer to value for the provided key. NULL if the key is not found.
 */

void *hashtable_get(struct t_hashtable *hashtable, const void *key)
{
	struct t_hashtable_item *item_ptr;

	item_ptr = hashtable_get_item(hashtable, key, NULL);

	return (item_ptr) ? item_ptr->value : NULL;
}

/*
 * Check the existing of the provided key.
 *
 * Returns:
 * 1: Key exists.
 * 0: Keys does not exist.
 */

int hashtable_has_key(struct t_hashtable *hashtable, const void *key)
{
	return (hashtable_get_item(hashtable, key, NULL) != NULL) ? 1 : 0;
}

/*
 * Convert a value (void *) to a string.
 *
 * Returns pointer to a static buffer, which will be overwritten by subsequent
 * calls to this function.
 */

const char *hashtable_to_string(enum t_hashtable_type type, const void *value)
{
	static char value_buff[128];

	switch (type) {
	case HASHTABLE_INTEGER:
		snprintf(value_buff, sizeof(value_buff), "%d", *((int *)value));
		return value_buff;
		break;
	case HASHTABLE_STRING:
		return (const char *)value;
		break;
	case HASHTABLE_POINTER:
	case HASHTABLE_NUM_TYPES:
		break;
	}
	return NULL;
}

/*
 * Map a function over all hashtable entries.
 */

void hashtable_fmap(struct t_hashtable *hashtable,
		    t_hashtable_fmap *callback_fmap, void *callback_fmap_data)
{
	struct t_hashtable_item *item_ptr, *item_next_ptr;
	int i;

	if (!hashtable)
		return;

	/* Silence */
	(void)item_next_ptr;

	for (i = 0; i < hashtable->size; ++i) {
		item_ptr = hashtable->htable[i];
		while (item_ptr) {
			item_next_ptr = item_ptr->next;
			(void)(callback_fmap)(hashtable, item_ptr->key,
					      item_ptr->value,
					      callback_fmap_data);

			item_ptr = item_next_ptr;
		}
	}
}

/*
 * Map a function over all hashtable entries and send keys and values as string.
 */

void hashtable_fmap_string(struct t_hashtable *hashtable,
			   t_hashtable_fmap_string *callback_fmap,
			   void *callback_fmap_data)
{
	struct t_hashtable_item *item_ptr, *item_next_ptr;
	const char *str_key, *str_value;
	char *key, *value;
	int i;

	if (!hashtable)
		return;

	/* Silence */
	(void)item_next_ptr;

	for (i = 0; i < hashtable->size; ++i) {
		item_ptr = hashtable->htable[i];
		while (item_ptr) {
			item_next_ptr = item_ptr->next;
			str_key = hashtable_to_string(hashtable->type_keys,
						      item_ptr->key);
			key = (str_key) ? string_strdup(str_key) : NULL;
			str_value = hashtable_to_string(hashtable->type_values,
							item_ptr->value);
			value = (str_value) ? string_strdup(str_value) : NULL;
			(void)(callback_fmap)(hashtable, key, value,
					      callback_fmap_data);

			if (key)
				free(key);

			if (value)
				free(value);
		}
	}
}

/*
 * Build a list of keys from the given hashtable.
 */

void hashtable_build_key_list_fmap_cb(struct t_hashtable *hashtable,
				      const void *key, const void *value,
				      void *data)
{
	struct t_pilist *list;

	/* silence! */
	(void)value;

	list = (struct t_pilist *)data;

	pilist_add(list, hashtable_to_string(hashtable->type_keys, key));
}

/*
 * Retrieve a list with the keys from the hashtable.
 *
 * This list has to be freed afterwards!
 *
 * Returns pointer to the list or NULL otherwise.
 */

struct t_pilist *hashtable_get_key_list(struct t_hashtable *hashtable)
{
	struct t_pilist *pilist;

	pilist = pilist_create();

	if (!pilist)
		return NULL;

	hashtable_fmap(hashtable, &hashtable_build_key_list_fmap_cb, pilist);

	return pilist;
}

/*
 * Build a list of values from the given hashtable.
 */

void hashtable_build_value_list_fmap_cb(struct t_hashtable *hashtable,
					const void *key, const void *value,
					void *data)
{
	struct t_pilist *list;

	/* silence! */
	(void)key;

	list = (struct t_pilist *)data;

	const char *string_value;
	string_value = hashtable_to_string(hashtable->type_values, value);

	/*
	 * string_value is null when pointer, or other invalid type, just pass
	 * along the reference in that case.
	 */
	(string_value) ? pilist_add(list, string_value) : pilist_add(list, value);
}

/*
 * Retrieve a list with the values from the hashtable.
 *
 * This list has to be freed afterwards!
 *
 * Returns pointer to the list or NULL otherwise.
 */

struct t_pilist *hashtable_get_value_list(struct t_hashtable *hashtable)
{
	struct t_pilist *pilist;

	pilist = pilist_create();

	if (!pilist)
		return NULL;

	hashtable_fmap(hashtable, &hashtable_build_value_list_fmap_cb, pilist);

	return pilist;
}

/*
 * Compute the length of all keys.
 */

void hashtable_compute_length_keys_cb(struct t_hashtable *hashtable,
				      const void *key, const void *value,
				      void *data)
{
	const char *str_key;
	int *len;

	/* Silence! */
	(void)value;

	len = (int *)data;

	str_key = hashtable_to_string(hashtable->type_keys, key);
	if (str_key)
		*len += strlen(str_key) + 1;
}

/*
 * Compute the length of all values.
 */

void hashtable_compute_length_values_cb(struct t_hashtable *hashtable,
					const void *key, const void *value,
					void *data)
{
	const char *str_value;
	int *len;

	/* Silence! */
	(void)key;

	len = (int *)data;

	if (value) {
		str_value = hashtable_to_string(hashtable->type_values, value);
		if (str_value)
			*len += strlen(str_value) + 1;
	} else {
		/* string representation of the null message of c */
		*len += strlen("(null)") + 1;
	}
}

/*
 * Compute length of all keys + values.
 */

void hashtable_compute_length_keys_value_cb(struct t_hashtable *hashtable,
					    const void *key, const void *value,
					    void *data)
{
	hashtable_compute_length_keys_cb(hashtable, key, value, data);
	hashtable_compute_length_values_cb(hashtable, key, value, data);
}

/*
 * Create a string with all the keys.
 */

void hashtable_build_string_keys_cb(struct t_hashtable *hashtable,
				    const void *key, const void *value,
				    void *data)
{
	const char *str_key;
	struct t_stringbuilder *sb_ptr;

	/* Silence! */
	(void)value;

	sb_ptr = (struct t_stringbuilder *)data;

	if (sb_ptr)
		stringbuilder_append(sb_ptr, ",");

	str_key = hashtable_to_string(hashtable->type_keys, key);
	if (str_key)
		stringbuilder_append(sb_ptr, str_key);
}

/*
 * Create a string with all the values.
 */

void hashtable_build_string_values_cb(struct t_hashtable *hashtable,
				      const void *key, const void *value,
				      void *data)
{
	const char *str_value;
	struct t_stringbuilder *sb_ptr;

	/* Silence! */
	(void)key;

	sb_ptr = (struct t_stringbuilder *)data;

	if (sb_ptr)
		stringbuilder_append(sb_ptr, ",");

	if (value) {
		str_value = hashtable_to_string(hashtable->type_values, value);
		if (str_value)
			stringbuilder_append(sb_ptr, str_value);
	} else {
		stringbuilder_append(sb_ptr, "(null)");
	}
}

/*
 * Create a string with all the key:values.
 */

void hashtable_build_string_keys_values_cb(struct t_hashtable *hashtable,
					   const void *key, const void *value,
					   void *data)
{
	const char *str_key, *str_value;
	struct t_stringbuilder *sb_ptr;

	sb_ptr = (struct t_stringbuilder *)data;

	if (sb_ptr)
		stringbuilder_append(sb_ptr, ",");

	str_key = hashtable_to_string(hashtable->type_keys, key);
	if (str_key)
		stringbuilder_append(sb_ptr, str_key);

	stringbuilder_append(sb_ptr, ":");

	if (value) {
		str_value = hashtable_to_string(hashtable->type_values, value);
		if (str_value)
			stringbuilder_append(sb_ptr, str_value);
	} else {
		stringbuilder_append(sb_ptr, "(null)");
	}
}

/*
 * Gets keys and/or values of hashtable as stringbuilder.
 *
 * Returns a stringbuilder with an internal string of these formats:
 * if keys == 1 and values == 0: "key1,key2,key3"
 * if keys == 0 and values == 1: "value1,value2,value3"
 * if keys == 1 and values == 1: "key1:value1,key2:value2,key3:value3"
 */

struct t_stringbuilder *hashtable_get_keys_values(struct t_hashtable *hashtable,
						  int keys, int values)
{
	struct t_pilist *key_list;
	struct t_pilist_node *node_ptr;
	struct t_stringbuilder *sb_ptr;
	int len;

	if (!hashtable)
		return NULL;

	/* discard old representation */
	if (hashtable->keys_values) {
		stringbuilder_free(hashtable->keys_values);
		hashtable->keys_values = NULL;
	}

	sb_ptr = stringbuilder_create();

	if (!sb_ptr)
		return NULL;

	hashtable->keys_values = sb_ptr;

	/* compute length of string */
	len = 0;
	hashtable_fmap(hashtable,
		       (keys && values) ?
			       &hashtable_compute_length_keys_value_cb :
			       ((keys) ? &hashtable_compute_length_keys_cb :
					 &hashtable_compute_length_values_cb),
		       &len);

	if (len == 0)
		return hashtable->keys_values;

	/* build string */
	if (keys) {
		key_list = hashtable_get_key_list(hashtable);
		if (key_list) {
			for (node_ptr = key_list->head; node_ptr;
			     node_ptr = node_ptr->next) {
				if (values) {
					hashtable_build_string_keys_values_cb(
						hashtable, node_ptr->data,
						hashtable_get(hashtable,
							      node_ptr->data),
						hashtable->keys_values);
				} else {
					hashtable_build_string_keys_cb(
						hashtable, node_ptr->data, NULL,
						hashtable->keys_values);
				}
			}
			pilist_free(key_list);
		}
	} else {
		hashtable_fmap(
			hashtable,
			(keys && values) ?
				&hashtable_build_string_keys_values_cb :
				((keys) ? &hashtable_build_string_keys_cb :
					  &hashtable_build_string_values_cb),
			hashtable->keys_values);
	}

	return hashtable->keys_values;
}

/*
 * Get a hashtable property as a string wrapped inside a stringbuilder.
 *
 * Returns a pointer to the stringbuilder holding the string of the property,
 * NULL otherwise.
 *
 * NOTE: The returned stringbuilder needs to be manually freed.
 */

struct t_stringbuilder *hashtable_get_string(struct t_hashtable *hashtable,
					     const char *property)
{
	struct t_stringbuilder *sb_ptr;

	sb_ptr = NULL;
	if (hashtable && property) {
		if (string_strcasecmp(property, "type_keys") == 0) {
			sb_ptr = stringbuilder_create();
			stringbuilder_append(
				sb_ptr,
				hashtable_type_string[hashtable->type_keys]);
		} else if (string_strcasecmp(property, "type_values") == 0) {
			sb_ptr = stringbuilder_create();
			stringbuilder_append(
				sb_ptr,
				hashtable_type_string[hashtable->type_values]);
		} else if (string_strcasecmp(property, "keys") == 0) {
			sb_ptr = hashtable_get_keys_values(hashtable, 1, 0);
		} else if (string_strcasecmp(property, "values") == 0) {
			sb_ptr = hashtable_get_keys_values(hashtable, 0, 1);
		} else if (string_strcasecmp(property, "keys_values") == 0) {
			sb_ptr = hashtable_get_keys_values(hashtable, 1, 1);
		}
	}

	return sb_ptr;
}

/*
 * Set a hashtable property (pointer)
 *
 * NOTE: Currently the properties that are allowed to be set:
 * - callback_free_key
 * - callback_free_value
 */

void hashtable_set_pointer(struct t_hashtable *hashtable, const char *property,
			   void *pointer)
{
	if (!hashtable || !property)
		return;

	if (string_strcasecmp(property, "callback_free_key") == 0)
		hashtable->callback_free_key = pointer;
	else if (string_strcasecmp(property, "callback_free_value") == 0)
		hashtable->callback_free_value = pointer;
}

/*
 * Remove an item from the hashtable.
 */

void hashtable_remove_item(struct t_hashtable *hashtable,
			   struct t_hashtable_item *item,
			   unsigned long long hash)
{
	if (!hashtable || !item)
		return;

	/* free value and key */
	hashtable_free_value(hashtable, item);
	hashtable_free_key(hashtable, item);

	/* remove item from the list */
	if (item->prev)
		(item->prev)->next = item->next;
	if (item->next)
		(item->next)->prev = item->prev;
	if (hashtable->htable[hash] == item)
		hashtable->htable[hash] = item->next;

	free(item);

	hashtable->count--;
}

/*
 * Remove item from hashtable (searches with key).
 */

void hashtable_remove(struct t_hashtable *hashtable, const void *key)
{
	struct t_hashtable_item *item_ptr;
	unsigned long long hash;

	if (!hashtable || !key)
		return;

	item_ptr = hashtable_get_item(hashtable, key, &hash);
	if (item_ptr)
		hashtable_remove_item(hashtable, item_ptr, hash);
}

/*
 * Remove all items from the hashtable.
 */

void hashtable_remove_all(struct t_hashtable *hashtable)
{
	if (!hashtable)
		return;

	for (int i = 0; i < hashtable->size; ++i)
		while (hashtable->htable[i])
			hashtable_remove_item(hashtable, hashtable->htable[i],
					      i);
}

/*
 * Free the hashtable.
 *
 * Remove all the items and deallocate the hashtable.
 */

void hashtable_free(struct t_hashtable *hashtable)
{
	if (!hashtable)
		return;

	hashtable_remove_all(hashtable);
	free(hashtable->htable);

	if (hashtable->keys_values)
		stringbuilder_free(hashtable->keys_values);
	free(hashtable->keys_values);
	free(hashtable);
}
