CFLAGS			=	-Wall -Wextra -Werror -O3

OBJ_DIR			=	obj

SRC_DIR 		=	src

SUB_SRC_DIR		=	glad\
					window\
					win_event\
					camera\
					render\
					texture\
					tinycthread\

ALL_SRC_DIR		=	$(OBJ_DIR) $(addprefix $(OBJ_DIR)/, $(SUB_SRC_DIR))

MAIN_MANDATORY 	=	main.c

SRCS			=	context_init.c\
					glad/gl.c\
					window/openGL_glw_init.c\
					win_event/key_callback.c\
					camera/camera.c\
					texture/load_texture.c\
					render/shader_utils.c\
					render/cube.c\
					render/occlusion_culling.c\
					render/chunks.c\
					render/render_chunks.c\
					render/skybox.c\
					render/load_chunks.c\
					render/thread_supervisor.c\
					render/thread_workers.c\
					render/frustrum_culling.c\
					render/cube_face_build.c\
					render/block.c\
					render/text_render.c\
					render/dig_cave.c\
					render/biome.c\

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

ifeq ($(findstring leak, $(MAKECMDGOALS)), leak)
CFLAGS = -Wall -Wextra -Werror -g3 -fsanitize=address
else ifeq ($(findstring thread, $(MAKECMDGOALS)), thread)
CFLAGS = -Wall -Wextra -Werror -g3 -fsanitize=thread
else ifeq ($(findstring debug, $(MAKECMDGOALS)), debug)
CFLAGS = -Wall -Wextra -Werror -g3
endif


