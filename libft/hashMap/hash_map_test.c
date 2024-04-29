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

void hash_map_colision_test() {
    // Initialize the hashmap
    t_hash_map *map = hash_map_init(4); // Small capacity to force collisions
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

    // Add more entries with keys that collide with existing entries
    t_block_pos pos4 = {13, 14, 15}; // Will collide with pos1, pos2, and pos3
    t_block_pos pos5 = {17, 18, 19}; // Will collide with pos1, pos2, and pos3
    assert(hash_map_set_entry(map, pos4, ft_strdup("Value4")) == HASH_MAP_ADD_ENTRY);
    assert(hash_map_set_entry(map, pos5, ft_strdup("Value5")) == HASH_MAP_ADD_ENTRY);

    // Retrieve values from the hashmap after adding more entries and verify
    assert(compare_values(hash_map_get(map, pos4), "Value4"));
    assert(compare_values(hash_map_get(map, pos5), "Value5"));

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


int main() {
    hash_map_test();
    printf("hash_map_test passed successfully!\n");
	hash_map_colision_test();
    printf("hash_map_colision_test successfully!\n");
	hash_map_expand_test();
	printf("hash_map_expand_test successfully!\n");
    return 0;
}
