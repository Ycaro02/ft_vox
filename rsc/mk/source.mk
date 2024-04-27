OBJ_DIR			=	obj

SRC_DIR 		=	src

SUB_SRC_DIR		=	window\
					win_event\
					glad\


ALL_SRC_DIR		=	$(OBJ_DIR) $(addprefix $(OBJ_DIR)/, $(SUB_SRC_DIR))

MAIN_MANDATORY 	=	main.c

SRCS			=	window/openGL_glw_init.c\
					win_event/key_callback.c\
					glad/gl.c\


# SRCS_BONUS		=	main_bonus.c

MAKE_LIBFT		=	make -s -C libft -j

MAKE_LIST		=	make -s -C libft/list -j

LIBFT			= 	libft/libft.a

LIST			= 	libft/list/linked_list.a

OBJS 			= $(addprefix $(OBJ_DIR)/, $(SRCS:.c=.o))

RM			=	rm -rf

ifeq ($(findstring bonus, $(MAKECMDGOALS)), bonus)
ASCII_NAME	= "bonus"
SRCS += $(SRCS_BONUS)
else
SRCS += $(MAIN_MANDATORY)
endif