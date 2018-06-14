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
typedef void(t_hashtable_free_value)(struct t_hashtable *hashtable,
				     const void *key, void *value);
typedef void(t_hashtable_fmap)(struct t_hashtable *hashtable, const void *key,
			       const void *value, void *data);
typedef void(t_hashtable_fmap_string)(struct t_hashtable *hashtable,
				      const void *key, const void *value,
				      void *data);

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
	 * A representation of the keys/values wrapped inside a stringbuilder.
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

#define PILAB_HASHTABLE_INTEGER "integer"
#define PILAB_HASHTABLE_STRING "string"
#define PILAB_HASHTABLE_POINTER "pointer"

extern int hashtable_get_type(const char *type);
extern unsigned long long hashtable_hash_key_djb2(const char *string);
extern unsigned long long
	hashtable_hash_key_default_cb(struct t_hashtable *hashtable,
				      const void *key);
extern int hashtable_keycmp_default_cb(struct t_hashtable *hashtable,
				       const void *key1, const void *key2);
extern struct t_hashtable *
	hashtable_create(int size, const char *type_keys,
			 const char *type_values,
			 t_hashtable_hash_key *callback_hash_key,
			 t_hashtable_keycmp *callback_keycmp);
extern void hashtable_alloc_type(enum t_hashtable_type type, const void *value,
				 void **pointer);
extern void hashtable_free_key(struct t_hashtable *hashtable,
			       struct t_hashtable_item *item);
extern void hashtable_free_value(struct t_hashtable *hashtable,
				 struct t_hashtable_item *item);
extern struct t_hashtable_item *hashtable_set(struct t_hashtable *hashtable,
					      const void *key,
					      const void *value);
extern struct t_hashtable_item *
	hashtable_get_item(struct t_hashtable *hashtable, const void *key,
			   unsigned long long *hash);
extern void *hashtable_get(struct t_hashtable *hashtable, const void *key);
extern int hashtable_has_key(struct t_hashtable *hashtable, const void *key);
extern const char *hashtable_to_string(enum t_hashtable_type type,
				       const void *value);
extern void hashtable_fmap(struct t_hashtable *hashtable,
			   t_hashtable_fmap *callback_fmap,
			   void *callback_fmap_data);
extern void hashtable_fmap_string(struct t_hashtable *hashtable,
				  t_hashtable_fmap_string *callback_fmap,
				  void *callback_fmap_data);
extern void hashtable_build_key_list_fmap_cb(struct t_hashtable *hashtable,
					     const void *key, const void *value,
					     void *data);
extern struct t_pilist *hashtable_get_key_list(struct t_hashtable *hashtable);
extern void hashtable_build_value_list_fmap_cb(struct t_hashtable *hashtable,
					       const void *key,
					       const void *value, void *data);
extern struct t_pilist *hashtable_get_value_list(struct t_hashtable *hashtable);
extern void hashtable_compute_length_keys_cb(struct t_hashtable *hashtable,
					     const void *key, const void *value,
					     void *data);
extern void hashtable_compute_length_values_cb(struct t_hashtable *hashtable,
					       const void *key,
					       const void *value, void *data);
extern void
	hashtable_compute_length_keys_value_cb(struct t_hashtable *hashtable,
					       const void *key,
					       const void *value, void *data);
extern void hashtable_build_string_keys_cb(struct t_hashtable *hashtable,
					   const void *key, const void *value,
					   void *data);
extern void hashtable_build_string_values_cb(struct t_hashtable *hashtable,
					     const void *key, const void *value,
					     void *data);
extern void hashtable_build_string_keys_values_cb(struct t_hashtable *hashtable,
						  const void *key,
						  const void *value,
						  void *data);
extern struct t_stringbuilder *
	hashtable_get_keys_values(struct t_hashtable *hashtable, int keys,
				  int values);
extern struct t_stringbuilder *
	hashtable_get_string(struct t_hashtable *hashtable,
			     const char *property);
extern void hashtable_set_pointer(struct t_hashtable *hashtable,
				  const char *property, void *pointer);
extern void hashtable_remove_item(struct t_hashtable *hashtable,
				  struct t_hashtable_item *item,
				  unsigned long long hash);
extern void hashtable_remove(struct t_hashtable *hashtable, const void *key);
extern void hashtable_remove_all(struct t_hashtable *hashtable);
extern void hashtable_free(struct t_hashtable *hashtable);

#endif
