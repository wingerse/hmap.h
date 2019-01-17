/*
 * Implements a generic hashmap.
 * Usage
 * =====
 * HMAP_DECLARE(K, V) 
 *     Defines structures hmap_K_V and hmap_K_V_entry, and declares the functions.
 *     If K or V is a pointer, then it has to be typedef'd.
 * HMAP_DEFINE(K, V, hash_func, eq_func) 
 *     Defines the functions. 
 *     hash_func: Must have signature: uint32_t hash_func(const K *)
 *     eq_func:   Must have signature: bool eq_func(const K *, const K *)
 * HMAP_ITER_BEGIN(h, element_name) 
 *     Starts a for loop where element_name is a pointer to hmap_K_V_entry which can be used as iterator value.
 *     Modifying the hashmap or entry except for the value is forbidden.
 * HMAP_ITER_END
 *     Ends the for loop 
 * There should not be any semicolon after the macros.
 * 
 * Functions
 * =========
 * void hmap_K_V_init_custom(hmap_K_V *h, float load_factor, uint32_t initial_capacity, void (*key_destructor)(K *key), void (*value_destructor)(V *value)): 
 *     Initiates the hashmap with given parameters. initial capacity is rounded to next power of 2. 
 *     Destructors can be NULL in which case they are ignored.
 *
 * void hmap_K_V_init(hmap_K_V *h, void (*key_destructor)(K *key), void (*value_destructor)(V *value)): 
 *     init_custom with default parameters + destructors forwarded.
 *
 * V *hmap_K_V_put(hmap_K_V *h, const K *key): 
 *     Puts the key, returning a pointer to the value. Allocates new entry if required.
 *
 * void hmap_K_V_put_entry(hmap_K_V *h, hmap_K_V_entry *entry):
 *     Puts an entry into the hashmap. If it already exists, the previous entry is destroyed.
 *
 * V *hmap_K_V_get(const hmap_K_V *h, const K *key): 
 *     Gets a pointer to the value associated with the key; returns NULL if it doesn't exist.
 *
 * hmap_K_V_entry *hmap_K_V_extract(hmap_K_V *h, const K *key): 
 *     Removes and returns the entry associated with the key; returns NULL if it doesn't exist.
 *     The entry has to be `free`d yourself. Destroying the key and value also is now your responsibility.
 *     The main use case is changing the key without reallocation - extract, change and then call put_entry.  
 *
 * bool hmap_K_V_remove(hmap_K_V *h, const K *key): 
 *     Removes the entry associated with the key from the map, freeing it and calling destructors for key and value.
 *     Returns true if removed, false if it doesn't exist.
 *
 * void hmap_K_V_destroy(hmap_K_V *h): 
 *     Destroys the map by freeing memory, and calling destructors of keys and values.
 * 
 * Example
 * =======
 * HMAP_DECLARE(int, int)
 * HMAP_DEFINE(int, int, hash_func, eq_func) // hash_func can be identity function. eq_func can use ==
 * 
 * hmap_int_int h;
 * hmap_int_int_init(&h, NULL, NULL);
 * *hmap_int_int_put(&h, &(int){1}) = 2;
 * printf("%d", *hmap_int_int_get(&h, &(int){1})); // 2
 * hmap_int_int_entry *e = hmap_int_int_extract(&h, &(int){1});
 * e->key = 2;
 * hmap_int_int_put_entry(&h, e);
 * printf("%d", *hmap_int_int_get(&h, &(int){2})); // 2 
 * hmap_int_int_remove(&h, &(int){2});
 * printf("%" PRIu32, h.len); // 0
 * hmap_int_int_destroy(&h); // memory freed. 
 */

#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>

#define HMAP_DEFAULT_LOAD_FACTOR      0.75
#define HMAP_DEFAULT_INITIAL_CAPACITY 16

#define HMAP_DECLARE(K, V) \
typedef struct hmap_##K##_##V##_entry hmap_##K##_##V##_entry;\
typedef struct hmap_##K##_##V##_entry {\
    uint32_t               hash;\
    K                      key;\
    V                      value;\
    hmap_##K##_##V##_entry *next;\
} hmap_##K##_##V##_entry;\
\
typedef struct hmap_##K##_##V {\
    uint32_t               len;\
    uint32_t               cap;\
    float                  load_factor;\
    uint32_t               threshold;\
    void                   (*key_destructor)(K *key);\
    void                   (*value_destructor)(V *value);\
    hmap_##K##_##V##_entry **buckets;\
} hmap_##K##_##V;\
\
void                    hmap_##K##_##V##_init_custom(hmap_##K##_##V *h, float load_factor, uint32_t initial_capacity, void (*key_destructor)(K *key), void (*value_destructor)(V *value));\
void                    hmap_##K##_##V##_init(hmap_##K##_##V *h, void (*key_destructor)(K *key), void (*value_destructor)(V *value));\
V                      *hmap_##K##_##V##_put(hmap_##K##_##V *h, const K *key);\
void                    hmap_##K##_##V##_put_entry(hmap_##K##_##V *h, hmap_##K##_##V##_entry *entry);\
V                      *hmap_##K##_##V##_get(const hmap_##K##_##V *h, const K *key);\
hmap_##K##_##V##_entry *hmap_##K##_##V##_extract(hmap_##K##_##V *h, const K *key);\
bool                    hmap_##K##_##V##_remove(hmap_##K##_##V *h, const K *key);\
void                    hmap_##K##_##V##_destroy(hmap_##K##_##V *h);

#define HMAP_ITER_BEGIN(h, element_name) \
for (uint32_t element_name##i = 0; element_name##i < (h)->cap; element_name##i++) {\
    typeof((h)->buckets[element_name##i]) element_name = (h)->buckets[element_name##i];\
    for (; element_name != NULL; element_name = element_name->next) {

#define HMAP_ITER_END \
    }\
}

#define HMAP_DEFINE(K, V, hash_func, eq_func)\
void hmap_##K##_##V##_init_custom(hmap_##K##_##V *h, float load_factor, uint32_t initial_capacity, void (*key_destructor)(K *key), void (*value_destructor)(V *value))\
{\
    h->len = 0;\
    uint32_t cap = 1;\
    while (cap < initial_capacity)\
        cap <<= 1;\
    h->cap = cap;\
    h->load_factor = load_factor;\
    h->threshold = load_factor * h->cap;\
    h->key_destructor = key_destructor;\
    h->value_destructor = value_destructor;\
    h->buckets = malloc(h->cap * sizeof(*h->buckets));\
    for (uint32_t i = 0; i < h->cap; i++) {\
        h->buckets[i] = NULL;\
    }\
}\
\
void hmap_##K##_##V##_init(hmap_##K##_##V *h, void (*key_destructor)(K *key), void (*value_destructor)(V *value))\
{\
    hmap_##K##_##V##_init_custom(h, HMAP_DEFAULT_LOAD_FACTOR, HMAP_DEFAULT_INITIAL_CAPACITY, key_destructor, value_destructor);\
}\
\
static uint32_t hmap_##K##_##V##_hash(const K *key) \
{\
    /* magic from jdk 7 hashmap. mitigates problems with power of 2 hashmap size*/\
    uint32_t h = hash_func(key);\
    h ^= (h >> 20) ^ (h >> 12);\
    return h ^ (h >> 7) ^ (h >> 4);\
}\
\
static void hmap_##K##_##V##_resize(hmap_##K##_##V *h) \
{\
    hmap_##K##_##V new = *h;\
    new.cap <<= 1;\
    new.threshold = new.load_factor * new.cap;\
    new.buckets = malloc(new.cap * sizeof(*new.buckets));\
    for (uint32_t i = 0; i < new.cap; i++) {\
        new.buckets[i] = NULL;\
    }\
\
    for (uint32_t i = 0; i < h->cap; i++) {\
        hmap_##K##_##V##_entry *e = (h)->buckets[i];\
        while(e != NULL) {\
            hmap_##K##_##V##_entry *next = e->next;\
            uint32_t new_hash = e->hash & (new.cap - 1);\
            e->next = new.buckets[new_hash];\
            new.buckets[new_hash] = e;\
            e = next;\
        }\
    }\
    free(h->buckets);\
    *h = new;\
}\
\
static void hmap_##K##_##V##_resize_if_required(hmap_##K##_##V *h)\
{\
    if (h->len >= h->threshold) {\
        hmap_##K##_##V##_resize(h);\
    }\
}\
\
V * hmap_##K##_##V##_put(hmap_##K##_##V *h, const K *key)\
{\
    hmap_##K##_##V##_resize_if_required(h);\
    uint32_t hash = hmap_##K##_##V##_hash(key);\
    uint32_t index = hash & (h->cap - 1);\
    hmap_##K##_##V##_entry **e = &h->buckets[index];\
    for (; *e != NULL; e = &(*e)->next) {\
        if ((*e)->hash == hash && eq_func(&(*e)->key, key)) {\
            return &(*e)->value;\
        }\
    }\
    hmap_##K##_##V##_entry *new_entry = malloc(sizeof(*new_entry));\
    new_entry->hash = hash;\
    new_entry->key = *key;\
    new_entry->next = NULL;\
    *e = new_entry;\
    h->len++;\
    return &new_entry->value;\
}\
\
void hmap_##K##_##V##_put_entry(hmap_##K##_##V *h, hmap_##K##_##V##_entry *entry)\
{\
    hmap_##K##_##V##_resize_if_required(h);\
    uint32_t hash = hmap_##K##_##V##_hash(&entry->key);\
    uint32_t index = hash & (h->cap - 1);\
    hmap_##K##_##V##_entry *e = h->buckets[index];\
	hmap_##K##_##V##_entry **prev_next = &h->buckets[index];\
	hmap_##K##_##V##_entry *next = NULL;\
	bool destroy = false;\
    for (; e != NULL; prev_next = &e->next, e = e->next) {\
        if (e->hash == hash && eq_func(&e->key, &entry->key)) {\
			next = e->next;\
			if (h->key_destructor != NULL) h->key_destructor(&e->key);\
			if (h->value_destructor != NULL) h->value_destructor(&e->value);\
			free(e);\
			destroy = true;\
			break;\
        }\
    }\
    entry->hash = hash;\
    entry->next = next;\
    *prev_next = entry;\
	if (!destroy)\
		h->len++;\
    return;\
}\
\
V *hmap_##K##_##V##_get(const hmap_##K##_##V *h, const K *key)\
{\
    uint32_t hash = hmap_##K##_##V##_hash(key);\
    uint32_t index = hash & (h->cap - 1);\
    hmap_##K##_##V##_entry *e = h->buckets[index];\
    for (; e != NULL; e = e->next) {\
        if (e->hash == hash && eq_func(&e->key, key)) {\
            return &e->value;\
        }\
    }\
    return NULL;\
}\
\
hmap_##K##_##V##_entry *hmap_##K##_##V##_extract(hmap_##K##_##V *h, const K *key)\
{\
    uint32_t hash = hmap_##K##_##V##_hash(key);\
    uint32_t index = hash & (h->cap - 1);\
    hmap_##K##_##V##_entry *e = h->buckets[index];\
    hmap_##K##_##V##_entry **prev_next = &h->buckets[index];\
    for (; e != NULL; prev_next = &e->next, e = e->next) {\
        if (e->hash == hash && eq_func(&e->key, key)) {\
            *prev_next = e->next;\
            h->len--;\
            return e;\
        }\
    }\
    return NULL;\
}\
\
bool hmap_##K##_##V##_remove(hmap_##K##_##V *h, const K *key)\
{\
    hmap_##K##_##V##_entry *entry = hmap_##K##_##V##_extract(h, key);\
    if (entry) {\
        if (h->key_destructor != NULL) h->key_destructor(&entry->key);\
        if (h->value_destructor != NULL) h->value_destructor(&entry->value);\
        free(entry);\
    }\
    return entry != NULL;\
}\
\
void hmap_##K##_##V##_destroy(hmap_##K##_##V *h)\
{\
    for (uint32_t i = 0; i < h->cap; i++) {\
        hmap_##K##_##V##_entry *e = h->buckets[i];\
        while (e != NULL) {\
            hmap_##K##_##V##_entry *next = e->next;\
            if (h->key_destructor != NULL) h->key_destructor(&e->key);\
            if (h->value_destructor != NULL) h->value_destructor(&e->value);\
            free(e);\
            e = next;\
        }\
    }\
    free(h->buckets);\
}
