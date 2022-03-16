NAME	= minishell
#NAME_BONUS	=

SRC_ELEM	=

#SRC_B_ELEM	=

SRC_DIR	= ./src/
#SRC_B_DIR	= ./src_bonus/

#FLAGS		= -lreadline -L/Users/$(USER)/.brew/Cellar/readline/8.1/lib/ -I/Users/$(USER)/.brew/Cellar/readline/8.1/include

FLAGS		= -lreadline -L/usr/local/opt/readline/lib/ -I/usr/local/opt/readline/include
LIBFT_DIR	= ./libft/
LIBFT		= $(LIBFT_DIR)libft.a

INCLUDES	= -I$(HEADER_DIR) -I$(HEADER_LIBFT)

INCLUDES_BONUS	= -I$(HEADER_B_DIR) -I$(HEADER_LIBFT)

HEADER_LIBFT= ./libft/includes/

HEADER_DIR	= ./includes/
HEADER_ELEM	= minishell.h

#HEADER_B_DIR	= ./includes_bonus/
#HEADER_B_ELEM	=

HEADER			= $(addprefix $(HEADER_DIR), $(HEADER_ELEM))
#HEADER_BONUS	= $(addprefix $(HEADER_B_DIR), $(HEADER_B_ELEM))

SRC	= $(addprefix $(SRC_DIR), $(SRC_ELEM))
#SRC_BONUS = $(addprefix $(SRC_B_DIR), $(SRC_B_ELEM))

OBJ_DIR		= obj/
OBJ_ELEM	= ${patsubst %.c, %.o, $(SRC_ELEM)}
OBJ = ${addprefix ${OBJ_DIR}, ${OBJ_ELEM}}

#OBJ_B_DIR	= obj_bonus/
#OBJ_B_ELEM	= ${patsubst %.c, %.o, $(SRC_B_ELEM)}
#OBJ_BONUS	= ${addprefix ${OBJ_B_DIR}, ${OBJ_B_ELEM}}

RM	= rm -rf
CFLAGS	= -Wall -Wextra -Werror

GREEN	= \033[0;32m
RED	= \033[0;31m
TXT	= \033[0m

.PHONY: all clean fclean re bonus

all: $(NAME)


${NAME}:	$(LIBFT) ${OBJ_DIR} ${OBJ}
			@clang ${CFLAGS} ${INCLUDES} ${OBJ} -o ${NAME}
			@echo "${RED}${NAME} compiling!${TXT}"
$(LIBFT):
			@$(MAKE) -sC $(LIBFT_DIR)

$(OBJ_DIR):
			@mkdir -p $(OBJ_DIR)
			
$(OBJ_DIR)%.o:$(SRC_DIR)%.c	${HEADER}
			@clang $(CFLAGS) -c $(INCLUDES) $< -o $@

#bonus: $(NAME_BONUS)
#${NAME_BONUS}:	$(LIBFT) ${OBJ_B_DIR} ${OBJ_BONUS}
#			@clang ${CFLAGS} ${INCLUDES_BONUS} ${OBJ_BONUS} -o ${NAME_BONUS}
#			@echo "${GREEN}${NAME_BONUS} compiling!${TXT}"
			
#$(OBJ_B_DIR):
#			@mkdir -p $(OBJ_B_DIR)
#
#$(OBJ_B_DIR)%.o:$(SRC_B_DIR)%.c	${HEADER_BONUS}
#			@clang $(CFLAGS) -c $(INCLUDES_BONUS) $< -o $@

clean:
			@$(MAKE) -sC $(LIBFT_DIR) clean
			@${RM} ${OBJ_DIR} ${OBJ_B_DIR}
			@echo "${RED}clean - ok!${TXT}"

fclean:	clean
			@${RM} $(LIBFT)
			@${RM} ${OBJ_DIR} ${NAME}
			@echo	"${GREEN}fclean - check!${TXT}"

re:		fclean all
