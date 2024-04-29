#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "hash_map.h"
#include "../libft.h"

// Function to compare block positions
int compare_block_pos(t_block_pos pos1, t_block_pos pos2) {
    return (pos1.x == pos2.x && pos1.y == pos2.y && pos1.z == pos2.z);
}

// Function to compare values
int compare_values(void *value1, void *value2) {
    // For this example, assuming values are strings
    return ft_strcmp((char *)value1, (char *)value2) == 0;
}

// Test function for the hashmap
void hash_map_test() {
    // Initialize the hashmap
    t_hash_map *map = hash_map_init(8);
    assert(map != NULL);

    // Add entries
    t_block_pos pos1 = {1, 2, 3};
    t_block_pos pos2 = {4, 5, 6};
    t_block_pos pos3 = {7, 8, 9};
    assert(hash_map_set_entry(map, pos1, ft_strdup("Value1")) == HASH_MAP_ADD_ENTRY);
    assert(hash_map_set_entry(map, pos2, ft_strdup("Value2")) == HASH_MAP_ADD_ENTRY);
    assert(hash_map_set_entry(map, pos3, ft_strdup("Value3")) == HASH_MAP_ADD_ENTRY);

    // Retrieve values from the hashmap and verify
    assert(compare_values(hash_map_get(map, pos1), "Value1"));
    assert(compare_values(hash_map_get(map, pos2), "Value2"));
    assert(compare_values(hash_map_get(map, pos3), "Value3"));
    assert(hash_map_get(map, (t_block_pos){10, 11, 12}) == NULL);

    // Add more entries after expansion
    assert(hash_map_set_entry(map, (t_block_pos){10, 11, 12}, ft_strdup("Value4")) == HASH_MAP_ADD_ENTRY);
    assert(hash_map_set_entry(map, (t_block_pos){13, 14, 15}, ft_strdup("Value5")) == HASH_MAP_ADD_ENTRY);

    // Retrieve values from the hashmap after expansion and verify
    assert(compare_values(hash_map_get(map, (t_block_pos){10, 11, 12}), "Value4"));
    assert(compare_values(hash_map_get(map, (t_block_pos){13, 14, 15}), "Value5"));

    // Update an entry
    assert(hash_map_set_entry(map, pos1, ft_strdup("UpdatedValue")) == HASH_MAP_UPT_ENTRY);
    assert(compare_values(hash_map_get(map, pos1), "UpdatedValue"));

    // Remove an entry
    assert(hash_map_set_entry(map, pos2, NULL) == HASH_MAP_UPT_ENTRY);
    assert(hash_map_get(map, pos2) == NULL);

    // Destroy the hashmap
    hash_map_destroy(map);
}

void hashmap_colision_test() {
    // Initialize the hashmap with a small capacity to force collisions
    t_hash_map *map = hash_map_init(4);
    assert(map != NULL);

    // Add entries with keys that collide
    assert(hash_map_set_entry(map, (t_block_pos){1, 2, 3}, ft_strdup("Value1")) == HASH_MAP_ADD_ENTRY);
    assert(hash_map_set_entry(map, (t_block_pos){5, 6, 7}, ft_strdup("Value2")) == HASH_MAP_ADD_ENTRY);
    assert(hash_map_set_entry(map, (t_block_pos){9, 10, 11}, ft_strdup("Value3")) == HASH_MAP_ADD_ENTRY);

    // Retrieve values from the hashmap and verify
    assert(compare_values(hash_map_get(map, (t_block_pos){1, 2, 3}), "Value1"));
    assert(compare_values(hash_map_get(map, (t_block_pos){5, 6, 7}), "Value2"));
    assert(compare_values(hash_map_get(map, (t_block_pos){9, 10, 11}), "Value3"));

    // Add more entries with keys that collide with existing entries
    assert(hash_map_set_entry(map, (t_block_pos){13, 14, 15}, ft_strdup("Value4")) == HASH_MAP_ADD_ENTRY);
    assert(hash_map_set_entry(map, (t_block_pos){17, 18, 19}, ft_strdup("Value5")) == HASH_MAP_ADD_ENTRY);
    assert(hash_map_set_entry(map, (t_block_pos){100, 108, 109}, ft_strdup("Value6")) == HASH_MAP_ADD_ENTRY);
    assert(hash_map_set_entry(map, (t_block_pos){117, 128, 129}, ft_strdup("Value7")) == HASH_MAP_ADD_ENTRY);

    // Retrieve values from the hashmap after adding more entries and verify
    assert(compare_values(hash_map_get(map, (t_block_pos){13, 14, 15}), "Value4"));
    assert(compare_values(hash_map_get(map, (t_block_pos){17, 18, 19}), "Value5"));
	assert(compare_values(hash_map_get(map, (t_block_pos){100, 108, 109}), "Value6"));
	assert(compare_values(hash_map_get(map, (t_block_pos){117, 128, 129}), "Value7"));

    // Destroy the hashmap
    hash_map_destroy(map);
}

void hash_map_expand_test() {
    // Initialize the hashmap
    t_hash_map *map = hash_map_init(3); // Small initial capacity
    assert(map != NULL);

    // Add entries
    t_block_pos pos1 = {1, 2, 3};
    t_block_pos pos2 = {5, 6, 7}; // Will collide with pos1
    t_block_pos pos3 = {9, 10, 11}; // Will collide with pos1 and pos2
    assert(hash_map_set_entry(map, pos1, ft_strdup("Value1")) == HASH_MAP_ADD_ENTRY);
    assert(hash_map_set_entry(map, pos2, ft_strdup("Value2")) == HASH_MAP_ADD_ENTRY);
    assert(hash_map_set_entry(map, pos3, ft_strdup("Value3")) == HASH_MAP_ADD_ENTRY);

    // Retrieve values from the hashmap and verify
    assert(compare_values(hash_map_get(map, pos1), "Value1"));
    assert(compare_values(hash_map_get(map, pos2), "Value2"));
    assert(compare_values(hash_map_get(map, pos3), "Value3"));

	assert(map->capacity == 3);
    // Expand the hashmap
    assert(hash_map_expand(map));
	
	assert(map->capacity == 6);

    // Add more entries after expansion
    t_block_pos pos4 = {13, 14, 15}; // No collision with existing entries after expansion
    t_block_pos pos5 = {17, 18, 19}; // No collision with existing entries after expansion
    assert(hash_map_set_entry(map, pos4, ft_strdup("Value4")) == HASH_MAP_ADD_ENTRY);
    assert(hash_map_set_entry(map, pos5, ft_strdup("Value5")) == HASH_MAP_ADD_ENTRY);

    // Retrieve values from the hashmap after adding more entries and verify
    assert(compare_values(hash_map_get(map, pos4), "Value4"));
    assert(compare_values(hash_map_get(map, pos5), "Value5"));


    // Destroy the hashmap
    hash_map_destroy(map);
}


// Test function for hashmap_length
void hashmap_length_test() {
    t_hash_map *map = hash_map_init(8);
    assert(map != NULL);

    assert(hashmap_length(map) == 0); // Initially, the length should be 0

    hash_map_set_entry(map, (t_block_pos){1, 2, 3}, ft_strdup("Value1"));
    assert(hashmap_length(map) == 1); // After adding one entry, the length should be 1

    hash_map_set_entry(map, (t_block_pos){4, 5, 6}, ft_strdup("Value2"));
    hash_map_set_entry(map, (t_block_pos){7, 8, 9}, ft_strdup("Value3"));
    assert(hashmap_length(map) == 3); // After adding two more entries, the length should be 3

    hash_map_destroy(map);
}

// Test function for hashmap_iterator and hashmap_next
void hashmap_iterator_test() {
    t_hash_map *map = hash_map_init(8);
    assert(map != NULL);

    hash_map_set_entry(map, (t_block_pos){1, 2, 3}, ft_strdup("Value1"));
    hash_map_set_entry(map, (t_block_pos){4, 5, 6}, ft_strdup("Value2"));
    hash_map_set_entry(map, (t_block_pos){7, 8, 9}, ft_strdup("Value3"));

    // Initialize the iterator
    t_hm_it it = hashmap_iterator(map);
    assert(it._map == map);
    assert(it._idx == 0);
    assert(it._current == NULL);

    // Test iterating through entries
    assert(hashmap_next(&it)); // Move to the first entry
	assert(it.key == hash_block_position(7, 8, 9));
    assert(strcmp((char *)it.value, "Value3") == 0);

	
    assert(hashmap_next(&it)); // Move to the first entry
    assert(it.key == hash_block_position(1, 2, 3));
    assert(strcmp((char *)it.value, "Value1") == 0);

    assert(hashmap_next(&it)); // Move to the third entry
    assert(it.key == hash_block_position(4, 5, 6));
    assert(strcmp((char *)it.value, "Value2") == 0);

    assert(!hashmap_next(&it)); // No more entries

    hash_map_destroy(map);
}

int main() {
    hash_map_test();
    printf("hash_map_test passed successfully!\n");
	hashmap_colision_test();
    printf("hash_map_colision_test successfully!\n");
	hash_map_expand_test();
	printf("hash_map_expand_test successfully!\n");
	hashmap_length_test();
	printf("test_hashmap_length passed successfully!\n");
	hashmap_iterator_test();
	printf("test_hashmap_iterator passed successfully!\n");
    return 0;
}
