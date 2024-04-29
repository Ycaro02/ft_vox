#ifndef HEADER_HASH_MAP_H
#define HEADER_HASH_MAP_H

#include "../libft.h"

typedef struct s_block_pos {
	u32 x;
	u32 y;
	u32 z;
} t_block_pos;

/* HashMap entry key + value */
typedef struct s_hash_map_entry {
	t_block_pos 	origin_data;	/* Original data (block pos) */
	u64				key;			/* Key of the entry */
	void 			*value;			/* Value linked */
} t_hm_entry;

/* HashMap structure */
typedef struct s_hash_map {
	t_list 		**entries;		/* array of t_list double ptr, head of each t_hm_entry list*/
	size_t		capacity;		/* Capacity of the array ( entry size )*/
	size_t		size;			/* Number of current item stored  */
} t_hash_map;

/* HashMap iterator struct */
typedef struct s_hash_map_it {
	u64			key;		/* Key of the current node */
	void 		*value;		/* Value of the current node */
	
	/* Fields get with hashMap iterator/next don't use these directly */
	t_hash_map	*_map;		/* HashMap head in this iterator */
	size_t		_idx;		/* Current index in the array */
} t_hm_it;


#define HASH_MAP_UPT_ENTRY		0U	/* Update hashmap entry */
#define HASH_MAP_ADD_ENTRY		1U	/* Add new entry */
#define HASH_MAP_MALLOC_ERROR	2U	/* Malloc error */

/* Hash Map entry.value is valid (not null) */
#define HASH_MAP_VALID_ENTRY(entry)	(entry->value != NULL)

/* Hash Map entry.key is the same as the key_cmp, same for x,y,z */
#define HASH_MAP_SAME_ENTRY(entry, key_cmp, x_cmp, y_cmp, z_cmp) (\
		entry->key == key_cmp &&\
        entry->origin_data.x == x_cmp &&\
        entry->origin_data.y == y_cmp &&\
        entry->origin_data.z == z_cmp\
)

/* Hash Map entry.key is the same as the key_cmp */
#define HASH_MAP_SAME_KEY(entry, key_cmp) (entry.key == key_cmp)


#define HASH_MAP_INDEX(key, capacity) (size_t)(key & (u64)(capacity - 1))

/**
 * @brief hash block coordinate to a unique key
 * @param x,y,z block coordinate
 * @return u64 key used to store the block in hashMap
*/
FT_INLINE u64 hash_block_position(u32 x, u32 y, u32 z) {
    u64 key = ((u64)x << 42) | ((u64)y << 21) | (u64)z;
    return (key);
}

/**
 * @brief HashMap init, create a new HashMap
 * @param capacity initial capacity of the HashMap
 * @return t_hash_map* new HashMap (NULL if failed)
*/
FT_INLINE t_hash_map *hash_map_init(size_t capacity) {
	t_hash_map *map = ft_calloc(sizeof(t_hash_map), 1);
	if (!map) {
		return (NULL);
	}

	map->entries = ft_calloc(sizeof(t_hm_entry), capacity);
	if (!map->entries) {
		free(map);
		return (NULL);
	}

	map->capacity = capacity;
	map->size = 0;
	return (map);
}

/**
 * @brief HashMap entry free, free the entry
 * @param entry entry to free
*/
FT_INLINE void hashmap_entry_free(void *entry) {
	t_hm_entry *e = (t_hm_entry *)entry;
	if (e->value) {
		free(e->value);
	}
	free(e);
}

/**
 * @brief HashMap destroy the map and free all the memory
 * @param map HashMap to destroy
*/
FT_INLINE void hash_map_destroy(t_hash_map *map) {
	if (!map) {
		return ;
	}
    for (size_t i = 0; i < map->capacity; i++) {
        ft_lstclear(&map->entries[i], hashmap_entry_free);
    }
	free(map->entries); /* free entry t_list ** array */
	free(map);			/* free map */
}

/**
 * @brief HashMap get the associated value with the key (x,y,z are the key)
 * @param map HashMap to search in
 * @param x,y,z key to search
 * @return void* value associated with the key (NULL if not found)
*/
FT_INLINE void *hash_map_get(t_hash_map *map, t_block_pos p) {
	u64		key = hash_block_position(p.x, p.y, p.z);
	// size_t	index = (size_t)(key & (u64)(map->capacity - 1));
	size_t	index = HASH_MAP_INDEX(key, map->capacity);

	t_list *entry = map->entries[index];
	while (entry) {
		t_hm_entry *e = (t_hm_entry *)entry->content;
		if (HASH_MAP_SAME_ENTRY(e, key, p.x, p.y, p.z)) {
			return (e->value);
		}
		entry = entry->next;
	}
	return (NULL);
}


/**
 * @brief HashMap set the value associated with the key (x,y,z are the key)
 * @param map HashMap to set in
 * @param x,y,z key to set
 * @param value value to set
 * @return u8 HASH_MAP_UPT_ENTRY if update, HASH_MAP_ADD_ENTRY if add, HASH_MAP_MALLOC_ERROR if malloc failed
*/
FT_INLINE u8 hash_map_set_entry(t_hash_map *map, t_block_pos p, void *value) 
{
	u64		key = hash_block_position(p.x, p.y, p.z);
	// size_t	index = (size_t)(key & (u64)(map->capacity - 1));
	size_t	index = HASH_MAP_INDEX(key, map->capacity);

	/* Check if the entry already exist */
	t_list *current = map->entries[index];
	while (current) {
		t_hm_entry *e = (t_hm_entry *)current->content;
		if (HASH_MAP_SAME_ENTRY(e, key, p.x, p.y, p.z)) {
			if (e->value) {
				free(e->value);
			}
			e->value = value;
			return (HASH_MAP_UPT_ENTRY);
		}
		current = current->next;
	}


	t_list *entry = ft_lstnew(ft_calloc(sizeof(t_hm_entry), 1));
	if (!entry) {
		return (HASH_MAP_MALLOC_ERROR);
	}
	t_hm_entry *e = (t_hm_entry *)entry->content;
	e->origin_data.x = p.x;
	e->origin_data.y = p.y;
	e->origin_data.z = p.z;
	e->key = key;
	e->value = value;
	ft_lstadd_back(&map->entries[index], entry);
	return (HASH_MAP_ADD_ENTRY);
}


FT_INLINE int hash_map_expand(t_hash_map *map) {
    size_t new_capacity = (map->capacity * 2); /* need to implement prime number check */
    t_list **new_entries = ft_calloc(sizeof(t_list *), new_capacity);
    if (!new_entries) {
        return (FALSE);
    }

    /* Rehash and move existing entries to the new array */
    for (size_t i = 0; i < map->capacity; i++) {
        t_list *current = map->entries[i];
        while (current) {
            t_hm_entry *entry = (t_hm_entry *)current->content;
            // size_t new_index = entry->key & (new_capacity - 1); // Calculate new index
            size_t new_index = HASH_MAP_INDEX(entry->key, new_capacity); // Calculate new index
            t_list *new_entry = ft_lstnew(entry);
            if (!new_entry) {
                // Handle memory allocation failure Free memory and return (FALSE)
                ft_lstclear(&new_entries[i], free);
                free(new_entries);
                return (FALSE);
            }
            ft_lstadd_back(&new_entries[new_index], new_entry);
            current = current->next;
        }
    }

    for (size_t i = 0; i < map->capacity; i++) {
		ft_lstclear_nodeptr(map->entries + i);
	}
    // Free old entries
    free(map->entries);
    
    // Update hashmap with new capacity and entries
    map->entries = new_entries;
    map->capacity = new_capacity;

    return (TRUE); // Expansion successful
}

#endif /* HEADER_HASH_MAP_H */