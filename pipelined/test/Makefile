
CC = gcc
PROGNAME = client
SRCS = client.c
OBJS = client.o

.c.o:
	rm -rf $@
	$(CC) -c $*.c

all: $(PROGNAME)

$(PROGNAME) : $(OBJS)
	$(CC) -o $(PROGNAME) $(OBJS) $(LIBS)

clean:
	rm -f $(OBJS) $(PROGNAME)
