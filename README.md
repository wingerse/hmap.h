# Implements a generic hashmap.  
  
Usage  
=====  
`HMAP_DECLARE(K, V) `  
    Defines structures `hmap_K_V` and `hmap_K_V_entry`, and declares the functions  
    If `K` or `V` is a pointer, then it has to be `typedef`d  
`HMAP_DEFINE(K, V, hash_func, eq_func)`  
    Defines the functions.  
    `hash_func`: Must have signature: `uint32_t hash_func(const K *)`  
    `eq_func`:   Must have signature: `bool eq_func(const K *, const K *)`  
`HMAP_ITER_BEGIN(h, element_name)`  
    Starts a for loop where `element_name` is a pointer to `hmap_K_V_entry` which can be used as iterator value.  
    Modifying the hashmap or entry except for the value is forbidden.  
`HMAP_ITER_END`  
    Ends the for loop  
There should not be any semicolon after the macros.  
  
Functions  
=========  
Prefix `hmap_K_V_` followed by:  
init_custom: Initiates the hashmap with given parameters and initial capacity is (rounded to next power of 2).  
             Destructors can be NULL in which case they are ignored.  
init       : Init_custom with default parameters  
put        : Puts the key, returning a pointer to the value. Allocates new entry if required  
put_entry  : Puts an entry into the hashmap if it doesn't exist. This entry has to come from extract method.  
get        : Gets a pointer to the value associated with the key; returns NULL if it doesn't exist.  
extract    : Removes and returns the entry associated with the key. The entry has to be `free`d yourself.   
             Destroying the key and value also is now your responsibility.  
             The main use case is changing they key without reallocation by calling put_entry after changing it.  
remove     : Removes the entry associated with the key from the map, freeing it and calling destructors for key and value.  
destroy    : Destroys the map by freeing memory, and calling destructors of keys and values.  
  
Example  
=======  

	HMAP_DECLARE(int, int)
	HMAP_DEFINE(int, int)

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
