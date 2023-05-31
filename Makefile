NAME = ft_nm
CC = gcc
CFLAGS = -Wall -Wextra -Werror -g -fsanitize=address
RM = rm -rf

SRC = main.c
OBJ = $(SRC:.c=.o)

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME)

re: fclean all

error_1:
	./ft_nm tester/errors/error_header
	
error_2:
	./ft_nm tester/errors/header_offset_error

error_3:
	./ft_nm tester/errors/unterminated_string

error_4:
	./ft_nm tester/errors/wrong_arch

easy_test_32bit:
	gcc -m32 -o easy_test_32bit ./tester/basics/easytest.c
	./ft_nm easy_test_32bit

easy_test_64bit:
	gcc -m64 -o easy_test_64bit ./tester/basics/easytest.c
	./ft_nm easy_test_64bit

not_so_easy_test_32bit:
	gcc -m32 -o easy_test_32bit ./tester/basics/notsoeasytest.c
	./ft_nm easy_test_32bit

not_so_easy_test_64bit:
	gcc -m64 -o notsoeasy_test_64bit ./tester/basics/notsoeasytest.c
	./ft_nm notsoeasy_test_64bit

.PHONY: all clean fclean re easy_test_32bit easy_test_64bit not_so_easy_test_32bit not_so_easy_test_64bit

