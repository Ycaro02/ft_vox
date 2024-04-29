#ifndef HEADER_HASHMAP_H
#define HEADER_HASHMAP_H

#include "../libft.h"

typedef struct s_block_pos {
	u32 x;
	u32 y;
	u32 z;
} t_block_pos;

/* HashMap entry key + value */
typedef struct s_hashmap_entry {
	t_block_pos 	origin_data;	/* Original data (block pos) */
	u64				key;			/* Key of the entry */
	void 			*value;			/* Value linked */
} t_hm_entry;

/* HashMap structure */
typedef struct s_hashmap {
	t_list 		**entries;		/* array of t_list double ptr, head of each t_hm_entry list*/
	size_t		capacity;		/* Capacity of the array ( entry size, number of list ptr )*/
	size_t		size;			/* Number of current item stored  */
} t_hashmap;

/* HashMap iterator struct */
typedef struct s_hashmap_it {
	u64			key;			/* Key of the current node */
	void 		*value;			/* Value of the current node */
	
	/* Fields get with hashMap iterator/next don't use these directly */
	t_hashmap	*_map;			/* HashMap head in this iterator */
	size_t		_idx;			/* Current index in the array */
	t_list		*_current;		/* Current node in the list */
} t_hm_it;


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

/**
 * @brief hash block coordinate to a unique key
 * @param x,y,z block coordinate
 * @return u64 KEY used to store the block in hashMap
*/
FT_INLINE u64 hash_block_position(u32 x, u32 y, u32 z) {
    u64 key = ((u64)x << 42) | ((u64)y << 21) | (u64)z;
    return (key);
}

/**
 * @brief HashMap init, create a new HashMap
 * @param capacity initial capacity of the HashMap
 * @return t_hashmap* new HashMap (NULL if failed)
*/
FT_INLINE t_hashmap *hashmap_init(size_t capacity) {
	t_hashmap *map = ft_calloc(sizeof(t_hashmap), 1);
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
FT_INLINE void hashmap_destroy(t_hashmap *map) {
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
FT_INLINE void *hashmap_get(t_hashmap *map, t_block_pos p) {
	u64		key = hash_block_position(p.x, p.y, p.z);
	size_t	index = HASHMAP_INDEX(key, map->capacity);

	t_list *entry = map->entries[index];
	while (entry) {
		t_hm_entry *e = (t_hm_entry *)entry->content;
		if (HASHMAP_SAME_ENTRY(e, key, p.x, p.y, p.z)) {
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
 * @return u8 HASHMAP_UPT_ENTRY if update, HASHMAP_ADD_ENTRY if add, HASHMAP_MALLOC_ERROR if malloc failed
*/
FT_INLINE u8 hashmap_set_entry(t_hashmap *map, t_block_pos p, void *value) 
{
	u64		key = hash_block_position(p.x, p.y, p.z);
	size_t	index = HASHMAP_INDEX(key, map->capacity);

	/* Check if the entry already exist */
	t_list *current = map->entries[index];
	while (current) {
		t_hm_entry *e = (t_hm_entry *)current->content;
		if (HASHMAP_SAME_ENTRY(e, key, p.x, p.y, p.z)) {
			if (e->value) {
				free(e->value);
			}
			e->value = value;
			return (HASHMAP_UPT_ENTRY);
		}
		current = current->next;
	}


	t_list *entry = ft_lstnew(ft_calloc(sizeof(t_hm_entry), 1));
	if (!entry) {
		return (HASHMAP_MALLOC_ERROR);
	}
	t_hm_entry *e = (t_hm_entry *)entry->content;
	e->origin_data.x = p.x;
	e->origin_data.y = p.y;
	e->origin_data.z = p.z;
	e->key = key;
	e->value = value;
	ft_lstadd_back(&map->entries[index], entry);
	(map->size)++;
	return (HASHMAP_ADD_ENTRY);
}


FT_INLINE int hashmap_expand(t_hashmap *map) {
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
            size_t new_index = HASHMAP_INDEX(entry->key, new_capacity); /* Calculate new index */
            t_list *new_entry = ft_lstnew(entry);
            if (!new_entry) {
                /* Handle memory allocation failure Free memory and return (FALSE) */
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
    /* Free old entries */
    free(map->entries);
    
    /* Update hashmap with new capacity and entries */
    map->entries = new_entries;
    map->capacity = new_capacity;

    return (TRUE); /* Expansion successful */
}

/* Function to get the length of the hash map */
FT_INLINE size_t hashmap_length(t_hashmap *map) {
    return (map->size);
}

/* Function to create and initialize an iterator for the hash map */
FT_INLINE t_hm_it hashmap_iterator(t_hashmap *map) {
    t_hm_it it;

    it._map = map;
    it._idx = 0;
	it._current = NULL;
    return (it);
}

/* Function to move to the next entry in the hash map */
FT_INLINE u8 hashmap_next(t_hm_it *it) {
    t_hashmap *map = it->_map;

    /* Loop through the entries array */
    while (it->_idx < map->capacity) {
        t_list *entry = map->entries[it->_idx];
        if (entry != NULL) {
            /* Found a non-empty list */
            if (it->_current == NULL) { /*  If it's the first node in the list, set it as the current node */
                it->_current = entry;
            } else { /* Otherwise, move to the next node in the list */
                it->_current = it->_current->next;
            }
            if (it->_current != NULL) {
                /* Found the next node in the list */
                t_hm_entry *hm_entry = (t_hm_entry *)it->_current->content;
                it->key = hm_entry->key;
                it->value = hm_entry->value;
                return (TRUE);
            }
        }
        (it->_idx)++;
        it->_current = NULL; /* Reset the list node pointer for the next iteration */
    }
    /* No more non-empty entries found */
    return (FALSE);
}

#endif /* HEADER_HASHMAP_H */