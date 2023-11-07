CFLAGS = -std=c11 -pedantic -pedantic-errors -g -Wall -Werror -Wextra -Wno-unused-parameter -Wno-newline-eof -Wno-implicit-fallthrough -D_POSIX_C_SOURCE=200112L -fsanitize=address -pthread
OBJS = main.o buffer.o netutils.o pop3cmd.o

CC = gcc

pop3d: $(OBJS)
	$(CC) $(CFLAGS) -o pop3d $(OBJS)

clean:
	rm -f pop3d $(OBJS)