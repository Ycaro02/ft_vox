/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   hashMap.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nfour <nfour@student.42angouleme.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/29 19:35:19 by nfour             #+#    #+#             */
/*   Updated: 2024/04/29 19:46:14 by nfour            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HEADER_HASHMAP_H
#define HEADER_HASHMAP_H

#include "../libft.h"

/**
 *	- Hash Map
 *	- A hash map is a data structure that implements an associative array abstract data type, a structure that can map keys to values.
 *	- A hash map uses a hash function to compute an index into an array of buckets or slots, from which the desired value can be found.
 *	- Ideally, the hash function will assign each key to a unique bucket, but most hash table designs employ an imperfect hash function, 
		which might cause hash collisions where the hash function generates the same index for more than one key.
 *	- In a well-dimensioned hash table, the average cost (number of instructions) for each lookup is independent of the number of elements stored in the table.
 *	- Many hash table designs also allow arbitrary insertions and deletions of key-value pairs, at (amortized) constant average cost per operation.
 *	- In many situations, hash tables turn out to be more efficient than search trees or any other table lookup structure.
 *	- For this implementation, we will use a simple hash function to hash the block position (x,y,z) to a unique key.
 *	- The hash map will be implemented with a simple array of linked list (chaining) to handle hash collisions.
 *	- The hash map will be able to store any type of data (void*), the user will have to handle the data type and give a free function to free the data.
 *	- The hash map will be able to get the length of the map.
 *	- The hash map will be able to set the value associated with the key (x,y,z).
 *	- The hash map will be able to get the value associated with the key (x,y,z).
 *	- The hash map will be able to destroy the map and free all the memory.
 *	- The hash map will be able to create and initialize an iterator for the hash map.
 *	- The hash map will be able to move to the next entry in the hash map.
 *	- The hash map will be able to expand the given hashmap capacity when needed.
 *	- The hash map will be able to update the entry if the key already exists.
 *  - API design from https://benhoyt.com/writings/hash-table-in-c/
 *  - t_list structure definition from ../libft.h
 *	typedef struct s_list {
 *		void			*content;
 *		struct s_list	*next;
 *	} t_list;
 *  - Content must be an allocated pointer on the heap, free will be called on each node with the free_obj function
*/

/* Block position structure */
typedef struct s_block_pos {
	u32 x;
	u32 y;
	u32 z;
} t_block_pos;

/* HashMap entry key + value + original data */
typedef struct s_hashmap_entry {
	t_block_pos 	origin_data;	/* Original data (block pos) */
	u64				key;			/* Key of the entry */
	void 			*value;			/* Value linked */
} hashMap_entry;

/* HashMap structure */
typedef struct s_hashmap {
	t_list 		**entries;				/* array of t_list double ptr, head of each hashMap_entry list*/
	size_t		capacity;				/* Capacity of the array ( entry size, number of list ptr )*/
	size_t		size;					/* Number of current item stored  */
	void		(*free_obj)(void *obj); /* Free function to free the given obj */
} hashMap;

/* HashMap iterator struct */
typedef struct s_hashmap_it {
	u64			key;			/* Key of the current node */
	void 		*value;			/* Value of the current node */
	
	/* Fields get with hashMap iterator/next don't use these directly */
	hashMap		*_map;			/* HashMap head in this iterator */
	size_t		_idx;			/* Current index in the array */
	t_list		*_current;		/* Current node in the list */
} hashMap_it;


#define HASHMAP_UPT_ENTRY		0U	/* Update hashmap entry */
#define HASHMAP_ADD_ENTRY		1U	/* Add new entry */
#define HASHMAP_MALLOC_ERROR	2U	/* Malloc error */


/* Hash Map entry.value is valid (not null) */
#define HASHMAP_VALID_ENTRY(entry)	(entry->value != NULL)

/* Hash Map entry.key is the same as the key_cmp, same for x,y,z */
#define HASHMAP_SAME_ENTRY(entry, key_cmp, x_cmp, y_cmp, z_cmp) (\
		entry->key == key_cmp &&\
        entry->origin_data.x == x_cmp &&\
        entry->origin_data.y == y_cmp &&\
        entry->origin_data.z == z_cmp\
)

/* Hash Map entry.key is the same as the key_cmp */
#define HASHMAP_SAME_KEY(entry, key_cmp) (entry.key == key_cmp)

/* Get hashmap index by key and capacity */
#define HASHMAP_INDEX(key, capacity) (size_t)(key & (u64)(capacity - 1))

/* HashMap function API */

/**
 * @brief hash block coordinate to a unique key
 * @param x,y,z block coordinate
 * @return u64 KEY used to store the block in hashMap
*/
u64 hash_block_position(u32 x, u32 y, u32 z);

/**
 * @brief HashMap init, create a new HashMap
 * @param capacity initial capacity of the HashMap
 * @return hashMap* new HashMap (NULL if failed)
*/
hashMap *hashmap_init(size_t capacity, void (*free_obj)(void *obj));

/**
 * @brief HashMap entry free, free the entry (give to parameter to hashMap init see implementation in hashMap.c)
 * @param entry entry to free
*/
void hashmap_entry_free(void *entry);

/**
 * @brief HashMap destroy the map and free all the memory
 * @param map HashMap to destroy
*/
void hashmap_destroy(hashMap *map);

/**
 * @brief HashMap get the associated value with the key (x,y,z are the key)
 * @param map HashMap to search in
 * @param x,y,z key to search
 * @return void* value associated with the key (NULL if not found)
*/
void *hashmap_get(hashMap *map, t_block_pos p);

/**
 * @brief HashMap set the value associated with the key (x,y,z are the key)
 * @param map HashMap to set in
 * @param x,y,z key to set
 * @param value value to set
 * @return u8 HASHMAP_UPT_ENTRY if update, HASHMAP_ADD_ENTRY if add, HASHMAP_MALLOC_ERROR if malloc failed
*/
u8 hashmap_set_entry(hashMap *map, t_block_pos p, void *value);

/**
 * @brief Function to get the length of the hash map
 * @param map HashMap to get the length
 * @return size_t length of the map
*/
size_t hashmap_length(hashMap *map);

/**
 * @brief Function to create and initialize an iterator for the hash map 
 * @param map HashMap to iterate
 * @return hashMap_it Iterator to the first entry in the map
*/
hashMap_it hashmap_iterator(hashMap *map);

/**
 *  @brief Function to move to the next entry in the hash map
 *	@param it Iterator to move
 *	@return int TRUE if move successful, FALSE otherwise (end of the map)
*/
s8 hashmap_next(hashMap_it *it);

/**
 * @brief Expand the given hashmap capacity
 * @param map HashMap to expand
 * @return int TRUE if expansion successful, FALSE otherwise
*/
s8 hashmap_expand(hashMap *map);

#endif /* HEADER_HASHMAP_H */