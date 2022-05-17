NAME				=	ft_ping

DIRECTORY_SOURCE	=	src
DIRECTORY_OBJECT	=	objects

FILES_CHECK			=	ft_ping.h
FILES_SOURCE		=	ft_ping.c

COMPILE				=	gcc
COMPILE				+=	-Wall -Wextra -Werror
COMPILE				+=	-g3 -ofast
ARGS				=	google.com

all: $(NAME)

$(DIRECTORY_OBJECT):
	@printf "\e[1;33mCreating directory: \e[0m$(DIRECTORY_OBJECT)\n"
	mkdir -p $@
	@printf "\e[1F\e[2K\r"

$(DIRECTORY_OBJECT)/%.o: $(DIRECTORY_SOURCE)/%.c $(FILES_CHECK:%=$(DIRECTORY_SOURCE)/%)
	@printf "\e[1;33mCompiling: \e[0m$<\n"
	$(COMPILE) $(COMPILATION_FLAG) -c $< -o $@
	@printf "\e[1F\e[2K\r"

$(NAME): $(DIRECTORY_OBJECT) $(FILES_SOURCE:%.c=$(DIRECTORY_OBJECT)/%.o)
	@printf "\e[1;33mLinking: \e[0m$@\n"
	$(COMPILE) -o $@ $(FILES_SOURCE:%.c=$(DIRECTORY_OBJECT)/%.o)
	@printf "\e[1F\e[2K\r\e[1;32mCompilation successful :D\e[0m\n"

run: all
	@printf "\e[1;34mRunning...\e[0m\n"
	$(RUN) ./$(NAME) $(ARGS)

clean:
	@printf "\e[1;31mCleaning: \e[0m$(DIRECTORY_OBJECT)\n"
	rm -rf $(DIRECTORY_OBJECT)
	@printf "\e[1F\e[2K\r\e[1;31mCleaned!\e[0m\n"

fclean: clean
	@printf "\e[1F\e[2K\r\e[1;31mCleaning: \e[0m$(NAME)\n"
	rm -f $(NAME)
	@printf "\e[1F\e[2K\r\e[1;31mCleaned!\e[0m\n"

re: fclean all

valgrind: all
	@printf "\e[1;33mChanging run mode to valgrind\e[0m\n"
	@make RUN="valgrind --leak-check=full --show-leak-kinds=all" run

fsanitize: fclean
	@printf "\e[1;33mChanging compile mode to fsanitize\e[0m\n"
	@make COMPILE="$(COMPILE) -fsanitize=address" run fclean

.PHONY: all run clean fclean re valgrind fsanitize
.SILENT:
