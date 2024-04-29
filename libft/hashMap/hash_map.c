#include <stdio.h>
#include "hash_map.h"

int main() {
    // Initialize a hashmap with initial capacity of 8
    t_hash_map *map = hash_map_init(8);
    if (map == NULL) {
        fprintf(stderr, "Failed to initialize hashmap\n");
        return 1;
    }

	char *str1 = ft_strdup("Value1");
	char *str2 = ft_strdup("Value2");
	char *str3 = ft_strdup("Value3");


	u8 ret = 0;
    // Add some entries to the hashmap
    ret = hash_map_set_entry(map, 1, 2, 3, str1);
    ft_printf_fd(1, "ret: %u\n", ret);
	hash_map_set_entry(map, 4, 5, 6, str2);
    hash_map_set_entry(map, 7, 8, 9, str3);

    // Retrieve values from the hashmap
    printf("Value at key (1, 2, 3): %s\n", (char *)hash_map_get(map, 1, 2, 3));
    printf("Value at key (4, 5, 6): %s\n", (char *)hash_map_get(map, 4, 5, 6));
    printf("Value at key (7, 8, 9): %s\n", (char *)hash_map_get(map, 7, 8, 9));
    printf("Value at key (10, 11, 12): %s\n", (char *)hash_map_get(map, 10, 11, 12)); // This should return NULL

    // Expand the hashmap and check if it's successful
    if (hash_map_expand(map)) {
        printf("Hashmap expanded successfully\n");
    } else {
        fprintf(stderr, "Failed to expand hashmap\n");
    }

    // Cleanup - free memory
    hash_map_destroy(map);

    return 0;
}
