
CC = gcc
PROGNAME = pipelined
SRCS = main.c options.c syslog.c modules.c muxcom.c
OBJS = main.o options.o syslog.o modules.o muxcom.o
CFLAGS = -g -Wall -L/usr/include -rdynamic -ldl
# sql.c -lmysqlclient
.c.o:
	rm -rf $@
	$(CC) -c $*.c

all: $(PROGNAME)

$(PROGNAME) : $(OBJS)
	$(CC) $(CFLAGS) -o $(PROGNAME) $(OBJS) $(LIBS)
	make -C ./plugins/
	make -C ./test/

clean:
	rm -f $(OBJS) $(PROGNAME)
	make -C ./plugins/ clean
	make -C ./test/ clean
