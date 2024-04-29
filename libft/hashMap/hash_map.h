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
	t_block_pos 				origin_data;	/* Original data (block pos) */
	u64							key;			/* Key of the entry */
	void 						*value;			/* Value linked */
	struct s_hash_map_entry 	*next;			/* Next entry in case of collision */
} t_hm_entry;

/* HashMap structure */
typedef struct s_hash_map {
	t_hm_entry	*entries;		/* Array of entries */
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


t_hm_it hash_map_iterator(t_hash_map *map)
{
	t_hm_it it;
	it._map = map;
	it._idx = 0;
	return (it);
}

u8 hash_map_it_next(t_hm_it *it)
{
	t_hash_map *map = it->_map;
	while (it->_idx < map->capacity) {
		size_t i = it->_idx;
		it->_idx += 1;
		t_hm_entry *entry = map->entries + i;
		if (HASH_MAP_VALID_ENTRY(entry)) {
			it->key = entry->key;
			it->value = entry->value;
			return (TRUE);
		}
	}
	return (FALSE);
}


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
	t_hash_map *map = (t_hash_map *)malloc(sizeof(t_hash_map));
	if (!map) {
		return (NULL);
	}

	map->entries = (t_hm_entry *)ft_calloc(sizeof(t_hm_entry), capacity);
	if (!map->entries) {
		free(map);
		return (NULL);
	}
	map->capacity = capacity;
	map->size = 0;
	return (map);
}

FT_INLINE void hash_map_entry_list_destroy(t_hm_entry *entry) {
    t_hm_entry *current = entry;

    while (current != NULL) {
        t_hm_entry *next = current->next;
        free(current);
        current = next;
    }
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
        hash_map_entry_list_destroy(map->entries + i);
    }
	free(map->entries);
	free(map);
}

/**
 * @brief HashMap get the associated value with the key (x,y,z are the key)
 * @param map HashMap to search in
 * @param x,y,z key to search
 * @return void* value associated with the key (NULL if not found)
*/
FT_INLINE void *hash_map_get(t_hash_map *map, u32 x, u32 y, u32 z) {
	u64			key = hash_block_position(x, y, z);
	size_t 		index = (size_t)(key & (u64)(map->capacity - 1));
	t_hm_entry	*entry = map->entries + index;

	ft_printf_fd(1, "map size %u, capacity: %u\n", map->size, map->capacity);

	while (HASH_MAP_VALID_ENTRY(entry)) {
		ft_printf_fd(1, "Iterate key %u\n", key);
		entry = map->entries + index;		
		while (entry != NULL) {
			if (HASH_MAP_SAME_ENTRY(entry, key, x, y, z)){
				ft_printf_fd(1, "Found key %u\n", key);
				return (entry->value);
			}
			entry = entry->next;
		}
		index = (index + 1) & (map->capacity - 1);
	}
	return (NULL);
}

#define HASH_MAP_UPT_ENTRY		0U	/* Update hashmap entry */
#define HASH_MAP_ADD_ENTRY		1U	/* Add new entry */
#define HASH_MAP_MALLOC_ERROR	2U	/* Malloc error */

/**
 * @brief HashMap set the value associated with the key (x,y,z are the key)
 * @param map HashMap to set in
 * @param x,y,z key to set
 * @param value value to set
 * @return u8 HASH_MAP_UPT_ENTRY if update, HASH_MAP_ADD_ENTRY if add, HASH_MAP_MALLOC_ERROR if malloc failed
*/
FT_INLINE u8 hash_map_set_entry(t_hash_map *map, u32 x, u32 y, u32 z, void *value) {
    u64		key = hash_block_position(x, y, z);
    size_t	index = (size_t)(key & (u64)(map->capacity - 1));

	/* Get linked list entry head */
    t_hm_entry *entry = map->entries + index;
    t_hm_entry *prev_entry = NULL;
    while (entry->next != NULL) {
        if (HASH_MAP_SAME_ENTRY(entry, key, x, y, z) == 1) {
            /* If same node update it and return */
            entry->value = value;
            return (HASH_MAP_UPT_ENTRY);
        }
        prev_entry = entry;
        entry = entry->next;
    }

	/* If no same node found create new entry */
    t_hm_entry *new_entry = (t_hm_entry *)malloc(sizeof(t_hm_entry));
    if (new_entry == NULL) {
        return (HASH_MAP_MALLOC_ERROR);
    }
    new_entry->origin_data.x = x;
    new_entry->origin_data.y = y;
    new_entry->origin_data.z = z;
    new_entry->key = key;
    new_entry->value = value;
    new_entry->next = NULL;

    /* If head no null add node at the end */
    if (prev_entry != NULL) {
		ft_printf_fd(1, "Not null not create lst node key %u, index %u\n", key, index);
        ft_printf_fd(1, "Prev entry = %u %u %u\n", prev_entry->origin_data.x, prev_entry->origin_data.y, prev_entry->origin_data.z);
		prev_entry->next = new_entry;
    } else { /* If head is null add node at the head */
        map->entries[index] = *new_entry;
		ft_printf_fd(1, "Create lst node %u\n", key);
    }
	/* Increment hashmap size */
	map->size++;
    return (HASH_MAP_ADD_ENTRY);
}


/**
 *	@brief Try to expand the hashmap
 *	@param map HashMap to expand
 *	@return u8 TRUE if success, FALSE if failed (out of memory)
*/
FT_INLINE u8 hash_map_expand(t_hash_map *map) {
    /* Calculate new capacity (double the current capacity) */
    size_t new_capacity = map->capacity * 2;

    /* Check for potential overflow */
    if (new_capacity < map->capacity) {
        return (FALSE); /* Overflow, capacity would be too big */
    }

    /* Allocate memory for the new entries array */
    t_hm_entry *new_entries = (t_hm_entry *)ft_calloc(new_capacity, sizeof(t_hm_entry));
    if (new_entries == NULL) {
        return (FALSE); /* Memory allocation failed */
    }

    /* Iterate through the existing entries, rehash them, and insert into the new array */
    for (size_t i = 0; i < map->capacity; i++) {
        t_hm_entry *entry = &map->entries[i];
        if (HASH_MAP_VALID_ENTRY(entry)) {
            /* Calculate new index using the new capacity */
            size_t new_index = entry->key & (new_capacity - 1);

            /* Insert the entry into the new array */
            if (new_entries[new_index].value != NULL) {
                /* Handle collision by chaining entries */
                t_hm_entry *current = &new_entries[new_index];
                while (current->next != NULL) {
                    current = current->next;
                }
                current->next = entry;
            } else {
                /* No collision, directly assign the entry to the new index */
                new_entries[new_index] = *entry;
            }
        }
    }

    /* Free the old entries array and update the map details */
    free(map->entries);
    map->entries = new_entries;
    map->capacity = new_capacity;

    return (TRUE);
}

#endif /* HEADER_HASH_MAP_H */