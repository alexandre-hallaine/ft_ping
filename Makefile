NAME				=	ft_ping

FILES_SOURCE		=	main.c

COMPILE				=	gcc
ARGS				=	ipv4.google.com

all: $(NAME)

$(NAME): $(FILES_SOURCE:.c=.o)
	$(COMPILE) -o $@ $(FILES_SOURCE:.c=.o)

run: all
	sudo ./$(NAME) $(ARGS)

clean:
	rm -rf $(FILES_SOURCE:.c=.o)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all run clean fclean re
