```
Implements a generic hashmap.
Usage
=====
HMAP_DECLARE(K, V) 
    Defines structures hmap_K_V and hmap_K_V_entry, and declares the functions.
    If K or V is a pointer, then it has to be typedef'd.
HMAP_DEFINE(K, V, hash_func, eq_func) 
    Defines the functions. 
    hash_func: Must have signature: uint32_t hash_func(const K *)
    eq_func:   Must have signature: bool eq_func(const K *, const K *)
HMAP_ITER_BEGIN(h, element_name) 
    Starts a for loop where element_name is a pointer to hmap_K_V_entry which can be used as iterator value.
    Modifying the hashmap or entry except for the value is forbidden.
HMAP_ITER_END
    Ends the for loop 
There should not be any semicolon after the macros.

Functions
=========
void hmap_K_V_init_custom(hmap_K_V *h, float load_factor, uint32_t initial_capacity, void (*key_destructor)(K *key), void (*value_destructor)(V *value)): 
    Initiates the hashmap with given parameters. initial capacity is rounded to next power of 2. 
    Destructors can be NULL in which case they are ignored.

void hmap_K_V_init(hmap_K_V *h, void (*key_destructor)(K *key), void (*value_destructor)(V *value)): 
    init_custom with default parameters + destructors forwarded.

V *hmap_K_V_put(hmap_K_V *h, const K *key): 
    Puts the key, returning a pointer to the value. Allocates new entry if required.

void hmap_K_V_put_entry(hmap_K_V *h, hmap_K_V_entry *entry):
    Puts an entry into the hashmap. If it already exists, the previous entry is destroyed.

V *hmap_K_V_get(const hmap_K_V *h, const K *key): 
    Gets a pointer to the value associated with the key; returns NULL if it doesn't exist.

hmap_K_V_entry *hmap_K_V_extract(hmap_K_V *h, const K *key): 
    Removes and returns the entry associated with the key; returns NULL if it doesn't exist.
    The entry has to be `free`d yourself. Destroying the key and value also is now your responsibility.
    The main use case is changing the key without reallocation - extract, change and then call put_entry.  

bool hmap_K_V_remove(hmap_K_V *h, const K *key): 
    Removes the entry associated with the key from the map, freeing it and calling destructors for key and value.
    Returns true if removed, false if it doesn't exist.

void hmap_K_V_destroy(hmap_K_V *h): 
    Destroys the map by freeing memory, and calling destructors of keys and values.

Example
=======
HMAP_DECLARE(int, int)
HMAP_DEFINE(int, int, hash_func, eq_func) // hash_func can be identity function. eq_func can use ==

hmap_int_int h;
hmap_int_int_init(&h, NULL, NULL);
*hmap_int_int_put(&h, &(int){1}) = 2;
printf("%d", *hmap_int_int_get(&h, &(int){1})); // 2
hmap_int_int_entry *e = hmap_int_int_extract(&h, &(int){1});
e->key = 2;
hmap_int_int_put_entry(&h, e);
printf("%d", *hmap_int_int_get(&h, &(int){2})); // 2 
hmap_int_int_remove(&h, &(int){2});
printf("%" PRIu32, h.len); // 0
hmap_int_int_destroy(&h); // memory freed. 
```
