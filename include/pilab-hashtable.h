#ifndef _PILAB_HASHTABLE_H
#define _PILAB_HASHTABLE_H
#include <unistd.h>
#include "pilab-stringbuilder.h"

struct t_hashtable;

typedef unsigned long long(t_hashtable_hash_key)(struct t_hashtable *hashtable,
						 const void *key);
typedef int(t_hashtable_keycmp)(struct t_hashtable *hashtable, const void *key1,
				const void *key2);
typedef void(t_hashtable_free_key)(struct t_hashtable *hashtable, void *key);

typedef void(t_hashtable_free_value)(struct t_hashtable *hashtable, const void *key,
				     void *value);
typedef void(t_hashtable_fmap)(struct t_hashtable *hashtable, const void *key,
			       const void *value, void *data);
typedef void(t_hashtable_fmap_string)(struct t_hashtable *hashtable, const void *key,
				      const void *value, void *data);

enum t_hashtable_type {
	HASHTABLE_INTEGER = 0,
	HASHTABLE_STRING,
	HASHTABLE_POINTER,
	/*
	 * Number of hashtable types
	 */
	HASHTABLE_NUM_TYPES,
};

struct t_hashtable_item {
	/*
	 * Key can be any type specified in t_hashtable_type.
	 */
	void *key;
	/*
	 * Value can be any type specified in t_hashtable_type.
	 */
	void *value;
	/*
	 * Size of key (in bytes).
	 */
	/* size_t key_size; */
	/*
	 * Size of value (in bytes).
	 */
	/* size_t value_size; */
	/*
	 * Pointer to the previous item.
	 */
	struct t_hashtable_item *prev;
	/*
	 * Pointer to the next item.
	 */
	struct t_hashtable_item *next;
};

struct t_hashtable {
	/*
	 * Size of the hashtable.
	 */
	int size;
	/*
	 * Number of items in hashtable.
	 */
	int count;
	/*
	 * Type for the key: integer/string/pointer.
	 */
	enum t_hashtable_type type_keys;
	/*
	 * Type for the value: integer/string/pointer.
	 */
	enum t_hashtable_type type_values;
	/*
	 * Internal array of the hashtable, consists of linked lists.
	 */
	struct t_hashtable_item **htable;
	/*
	 * A representation of the keys/values wrapped inside a strinbuilder.
	 */
	struct t_stringbuilder *keys_values;

	/* Callbacks */

	/*
	 * Hash key to int value.
	 */
	t_hashtable_hash_key *callback_hash_key;
	/*
	 * Compare two keys.
	 */
	t_hashtable_keycmp *callback_keycmp;
	/*
	 * Callback to free key.
	 */
	t_hashtable_free_key *callback_free_key;
	/*
	 * Callback to free value.
	 */
	t_hashtable_free_value *callback_free_value;
};

extern unsigned long long hashtable_hash_key_djb2(const char *string);
extern struct t_hashtable *hashtable_create(int size, const char *type_keys,
					    const char *type_values,
					    t_hashtable_hash_key *hash_key_cb,
					    t_hashtable_keycmp *keycmp_cb);

#endif
