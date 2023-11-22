CFLAGS = -std=c11 -pedantic -pedantic-errors -g -Wall -Werror -Wextra -Wno-unused-parameter -Wno-newline-eof -Wno-implicit-fallthrough -D_POSIX_C_SOURCE=200112L -fsanitize=address -pthread -O2
OBJS = main.o buffer.o netutils.o pop3cmd.o nuestro-parser.o command-handler.o args.o pop3file.o selector.o pop3.o ./manager/manager_handler.o ./manager/manager_parser.o metrics.o users.o stm.o


pop3d: $(OBJS)
	$(CC) $(CFLAGS) -o pop3d $(OBJS)

clean:
	rm -f pop3d $(OBJS)