include libft/rsc/mk/color.mk
include rsc/mk/source.mk

NAME			=	ft_vox
CC				=	clang

ASCII_ART		=	./rsc/sh/vox_ascii.sh

FREETYPE_INC	=	-Irsc/deps/freetype/include

LIB_DEPS_DIR	=	-Lrsc/lib_deps/

OPENGL_LIB		= -lglfw3 -lGL -lm -lstatic_tinycthread

# zlib1g-dev libbz2-dev libpng-dev libbrotli-dev
FREETYPE_LIB	= -lstaticfreetype -lz -lbz2 -lpng16 -lbrotlidec

INSTALL_DEPS	=	./install/install_deps.sh

DEPS_RULE		=	rsc/deps

all:		$(NAME)

$(NAME): $(DEPS_RULE) $(LIBFT) $(LIST) $(OBJ_DIR) $(OBJS)
	@${INSTALL_DEPS}
	@$(MAKE_LIBFT)
	@$(MAKE_LIST)
	@printf "$(CYAN)Compiling ${NAME} ...$(RESET)\n"
	@$(CC) $(CFLAGS) $(FREETYPE_INC) -o $(NAME) $(OBJS) $(LIBFT) $(LIST) $(LIB_DEPS_DIR) $(OPENGL_LIB) $(FREETYPE_LIB)
	@printf "$(GREEN)Compiling $(NAME) done$(RESET)\n"

$(DEPS_RULE):
	@$(INSTALL_DEPS)
	@printf "$(GREEN)Installing dependencies done$(RESET)\n"

$(LIST):
ifeq ($(shell [ -f ${LIST} ] && echo 0 || echo 1), 1)
	@printf "$(CYAN)Compiling list...$(RESET)\n"
	@$(MAKE_LIST)
	@printf "$(GREEN)Compiling list done$(RESET)\n"
endif

$(LIBFT):
ifeq ($(shell [ -f ${LIBFT} ] && echo 0 || echo 1), 1)
	@printf "$(CYAN)Compiling libft...$(RESET)\n"
	@$(MAKE_LIBFT)
	@printf "$(GREEN)Compiling libft done$(RESET)\n"
endif

$(OBJ_DIR):
	@mkdir $(ALL_SRC_DIR)
	@$(ASCII_ART)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@printf "$(YELLOW)Compile $<$(RESET) $(BRIGHT_BLACK)-->$(RESET) $(BRIGHT_MAGENTA)$@$(RESET)\n"
	@$(CC) $(CFLAGS) $(FREETYPE_INC) -o $@ -c $< $(CFLAGS)

bonus: clear_mandatory ${NAME}

clear_mandatory:
ifeq ($(shell [ -f ${OBJ_DIR}/main.o ] && echo 0 || echo 1), 0)
	@printf "$(RED)Clean mandatory obj $(RESET)\n"
	@rm -rf ${OBJ_DIR}
endif

clean:
ifeq ($(shell [ -d ${OBJ_DIR} ] && echo 0 || echo 1), 0)
	@$(RM) $(OBJ_DIR)
	@printf "$(RED)Clean $(OBJ_DIR)/test output$(RESET)\n"
	@$(RM)
endif

fclean:		clean_lib clean
	@printf "$(RED)Clean $(NAME)/lib$(RESET)\n"
	@$(RM) $(NAME)

clean_lib:
	@$(MAKE_LIST) fclean
	@$(MAKE_LIBFT) fclean

test: $(NAME)
	@./$(NAME)

# @ulimit -c unlimited
leak thread: clean $(NAME)
	@printf	"$(CYAN)CFLAGS: $(CFLAGS)$(RESET)\n"
	@./$(NAME)

re: clean $(NAME)

.PHONY:		all clean fclean re bonus
