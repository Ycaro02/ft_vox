/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HashMap.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nfour <nfour@student.42angouleme.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/29 19:35:27 by nfour             #+#    #+#             */
/*   Updated: 2024/05/23 16:42:45 by nfour            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HashMap.h"
#include "primeNumber.h"

/* Basic function you can provide to hashmap_init */
void hashmap_entry_free(void *entry) {
	HashMap_entry *e = (HashMap_entry *)entry;
	if (e->value) {
		free(e->value); /* replace this by your free function */
	}
	free(e); /* free the entry t_list node */
}

u64 hash_block_position(s32 x, s32 y, s32 z) {
    u64 key = ((u64)x << 42) | ((u64)y << 21) | (u64)z;
    return (key);
}

HashMap *hashmap_init(size_t capacity, void (*free_obj)(void *obj)) {
	HashMap *map = NULL;

	if (!free_obj) {
		ft_printf_fd(2, "hashmap_init: free_obj is NULL user need to provide free function\n");
		return (NULL);
	}
	
	map = ft_calloc(sizeof(HashMap), 1);
	if (!map) {
		return (NULL);
	}

	size_t prime_capacity = GET_NEXT_PRIME(capacity);

	map->entries = ft_calloc(sizeof(HashMap_entry), prime_capacity);
	if (!map->entries) {
		free(map);
		return (NULL);
	}

	mtx_init(&map->mtx, mtx_plain);
	map->capacity = prime_capacity;
	map->size = 0;
	map->free_obj = free_obj;
	return (map);
}

void hashmap_destroy(HashMap *map) {
	if (!map) {
		return ;
	}
    for (size_t i = 0; i < map->capacity; i++) {
        ft_lstclear(&map->entries[i], map->free_obj);
    }
	mtx_destroy(&map->mtx);
	free(map->entries); /* free entry t_list ** array */
	free(map);			/* free map */
}

void *hashmap_get(HashMap *map, BlockPos p) {
	u64		key = hash_block_position(p.x, p.y, p.z);
	size_t	index = HASHMAP_INDEX(key, map->capacity);
	t_list	*entry =  NULL;
	void	*ret = NULL;

	mtx_lock(&map->mtx);
	entry = map->entries[index];
	while (entry) {
		HashMap_entry *e = (HashMap_entry *)entry->content;
		if (HASHMAP_SAME_ENTRY(e, key, p.x, p.y, p.z)) {
			ret = e->value;
			break ;
		}
		entry = entry->next;
	}
	mtx_unlock(&map->mtx);
	return (ret);
}

FT_INLINE void hashmap_entry_update(HashMap_entry *dst, BlockPos p, u64 key, void *value) {
	dst->origin_data.x = p.x;
	dst->origin_data.y = p.y;
	dst->origin_data.z = p.z;
	dst->key = key;
	dst->value = value;
}

s8 hashmap_set_entry(HashMap *map, BlockPos p, void *value) {
	u64		key = hash_block_position(p.x, p.y, p.z);
	size_t	index = HASHMAP_INDEX(key, map->capacity);
	t_list	*current = NULL;
	HashMap_entry *newEntry = NULL;


	mtx_lock(&map->mtx);
	current = map->entries[index];
	while (current) {
		HashMap_entry *entry = (HashMap_entry *)current->content;
		if (HASHMAP_SAME_ENTRY(entry, key, p.x, p.y, p.z)) {
			map->free_obj(entry);
			if (!(newEntry = ft_calloc(sizeof(HashMap_entry), 1))) {
				mtx_unlock(&map->mtx);
				return (HASHMAP_MALLOC_ERROR);
			}
			hashmap_entry_update(newEntry, p, key, value);
			current->content = newEntry;
			mtx_unlock(&map->mtx);
			return (HASHMAP_UPT_ENTRY);
		}
		current = current->next;
	}

	t_list *entryNode = ft_lstnew(ft_calloc(sizeof(HashMap_entry), 1));
	if (!entryNode) {
		mtx_unlock(&map->mtx);
		return (HASHMAP_MALLOC_ERROR);
	}
	HashMap_entry *e = (HashMap_entry *)entryNode->content;
	hashmap_entry_update(e, p, key, value);
	ft_lstadd_back(&map->entries[index], entryNode);
	(map->size)++;
	mtx_unlock(&map->mtx);
	return (HASHMAP_ADD_ENTRY);
}

s8 hashmap_remove_entry(HashMap *map, BlockPos p, s8 free_data) {
    u64		key = hash_block_position(p.x, p.y, p.z);
    size_t	index = HASHMAP_INDEX(key, map->capacity);
    t_list	*current = NULL, *prev = NULL;
	s8		ret = HASHMAP_NOT_FOUND;

	mtx_lock(&map->mtx);
	current = map->entries[index];
    /* loop on linked list of the computed index */
    while (current) {
        HashMap_entry *entry = (HashMap_entry *)current->content;
        if (HASHMAP_SAME_ENTRY(entry, key, p.x, p.y, p.z)) {
			/* If is the first node of list update directly map entry, otherwise update prev->next */
			prev == NULL ? (map->entries[index] = current->next) : (prev->next = current->next); 
			/* If free data, free it otherwise just free entry struct */
			free_data == HASHMAP_FREE_DATA ? map->free_obj(entry) : free(entry);
			ret = free_data == HASHMAP_FREE_DATA ? HASHMAP_DATA_REMOVED : HASHMAP_ENTRY_FREE;
			/* free node and set it to NULL */
			free(current);
			current = NULL;
            (map->size)--;
			mtx_unlock(&map->mtx);
            return (ret);
        }
        prev = current;
        current = current->next;
    }
	mtx_unlock(&map->mtx);
	return (ret);
}



s8 hashmap_expand(HashMap *map) 
{
    size_t	new_capacity = 0;
    t_list	**new_entries = NULL;
	
	mtx_lock(&map->mtx);
	new_capacity = (map->capacity * 2);
	new_capacity = GET_NEXT_PRIME(new_capacity);
	/* Allocate new entries array */
    if (!(new_entries = ft_calloc(sizeof(t_list *), new_capacity))) {
        return (FALSE);
    }

    /* Rehash and move existing entries to the new array */
    for (size_t i = 0; i < map->capacity; i++) {
        t_list *current = map->entries[i];
        while (current) {
            HashMap_entry *entry = (HashMap_entry *)current->content;
            size_t new_index = HASHMAP_INDEX(entry->key, new_capacity); /* Calculate new index */
            t_list *new_entry = ft_lstnew(entry);
            if (!new_entry) {
                /* Handle memory allocation failure Free memory and return (FALSE) */
                ft_lstclear(&new_entries[i], free);
                free(new_entries);
				mtx_unlock(&map->mtx);
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

	mtx_unlock(&map->mtx);
    return (TRUE); /* Expansion successful */
}

size_t hashmap_size(HashMap *map) {
	size_t size;

	mtx_lock(&map->mtx);
    size = map->size;
	mtx_unlock(&map->mtx);
	return (size);
}

size_t hashmap_capacity(HashMap *map) {
	size_t capacity;

	mtx_lock(&map->mtx);
	capacity = map->capacity;
	mtx_unlock(&map->mtx);
	return (capacity);
}

HashMap_it hashmap_iterator(HashMap *map) {
    HashMap_it it;

	mtx_lock(&map->mtx);
    it._map = map;
    it._idx = 0;
	it._current = NULL;
	mtx_unlock(&map->mtx);
    return (it);
}

s8 hashmap_next(HashMap_it *it) {
    HashMap	*map = it->_map;
	t_list	*entry = NULL;

	mtx_lock(&map->mtx);

    /* Loop through the entries array */
    while (it->_idx < map->capacity) {
        entry = map->entries[it->_idx];
        if (entry != NULL) { /* Found a non-empty list */
            if (it->_current == NULL) { /*  If it's the first node in the list, set it as the current node */
                it->_current = entry;
            } else { /* Otherwise, move to the next node in the list */
                it->_current = it->_current->next;
            }
            if (it->_current != NULL) {
                /* Go to the next entry list */
                HashMap_entry *hm_entry = it->_current->content;
                it->key = hm_entry->key;
                it->value = hm_entry->value;
				mtx_unlock(&map->mtx);
                return (TRUE);
            }
        }
        (it->_idx)++;
        it->_current = NULL; /* Reset the list node pointer for the next iteration */
    }
    /* No more non-empty entries found */
	mtx_unlock(&map->mtx);
    return (FALSE);
}