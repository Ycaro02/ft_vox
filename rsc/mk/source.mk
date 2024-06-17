OBJ_DIR			=	obj

SRC_DIR 		=	src

SUB_SRC_DIR		=	glad\
					window\
					win_event\
					camera\
					render\
					texture\
					perlin_noise\
					tinycthread\

ALL_SRC_DIR		=	$(OBJ_DIR) $(addprefix $(OBJ_DIR)/, $(SUB_SRC_DIR))

MAIN_MANDATORY 	=	main.c

SRCS			=	context_init.c\
					window/openGL_glw_init.c\
					win_event/key_callback.c\
					glad/gl.c\
					camera/camera.c\
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
					texture/load_texture.c\
					perlin_noise/perlin_noise.c\
					perlin_noise/noise_utils.c\
					perlin_noise/noise_image.c\
					perlin_noise/noise_interpolate_utils.c\
					perlin_noise/snake_perlin.c\

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
