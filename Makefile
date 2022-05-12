NAME				=	ft_ping

DIRECTORY_SOURCE	=	src
DIRECTORY_OBJECT	=	objects

FILES_CHECK			=	ping.h
FILES_SOURCE		=	arg.c \
						header.c \
						init.c \
						signal.c \
						reply.c \
						main.c

COMPILE				=	gcc
COMPILE				+=	-Wall -Wextra -Werror
COMPILE				+=	-g3 -ofast
ARGS				=	-v google.com

all: $(NAME)

$(DIRECTORY_OBJECT)/%.o: $(DIRECTORY_SOURCE)/%.c $(FILES_CHECK:%=$(DIRECTORY_SOURCE)/%)
	@printf "\e[33m"
	@printf "Compile\t$< -> $@\n"
	mkdir -p $(dir $@)
	$(COMPILE) $(COMPILATION_FLAG) -c $< -o $@

$(NAME): $(FILES_SOURCE:%.c=$(DIRECTORY_OBJECT)/%.o)
	@printf "\e[32m"
	@printf "Build\t$@\n"
	$(COMPILE) -o $@ $(FILES_SOURCE:%.c=$(DIRECTORY_OBJECT)/%.o)

run: all
	@printf "\e[0m"
	$(RUN) ./$(NAME) $(ARGS)

clean:
	@printf "\e[31m"
	@printf "Remove\t$(DIRECTORY_OBJECT)\n"
	rm -rf $(DIRECTORY_OBJECT)

fclean: clean
	@printf "Remove\t$(NAME)\n"
	rm -f $(NAME)

re: fclean all

valgrind: all
	@make RUN="valgrind --leak-check=full --show-leak-kinds=all" run

fsanitize: fclean
	@make COMPILE="$(COMPILE) -fsanitize=address" run fclean

.PHONY: all run clean fclean re valgrind fsanitize
.SILENT:
