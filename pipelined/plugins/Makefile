
CC = gcc
PROGNAME = route
SRCS = route.c
CFLAGS = -g -Wall -shared

all: $(PROGNAME)

$(PROGNAME) : $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(PROGNAME)

clean:
	rm -f $(PROGNAME)
