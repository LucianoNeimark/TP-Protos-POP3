CFLAGS = -std=c11 -pedantic -pedantic-errors -g -Wall -Werror -Wextra -Wno-unused-parameter -Wno-newline-eof -Wno-implicit-fallthrough -D_POSIX_C_SOURCE=200112L -fsanitize=address -pthread -O2
SHARED_OBJS = ./utils/buffer.o ./utils/netutils.o socket_setup.o ./logger/logger.o
SERVER_OBJS = $(SHARED_OBJS) main.o pop3.o pop3cmd.o pop3file.o ./utils/stm.o parser.o command_handler.o ./utils/args.o users.o ./utils/selector.o metrics.o ./manager/manager_handler.o ./manager/manager_parser.o 
CLIENT_OBJS = $(SHARED_OBJS) manager_client.o

pop3d: $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o pop3d $(SERVER_OBJS)

mc: $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o manager_client $(CLIENT_OBJS)

clean:
	rm -f pop3d manager_client $(SERVER_OBJS) $(CLIENT_OBJS)