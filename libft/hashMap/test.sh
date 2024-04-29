#!/bin/bash

gcc hash_map_test.c ../libft.a ../list/linked_list.a -g && valgrind --leak-check=full ./a.out
rm -rf a.out
