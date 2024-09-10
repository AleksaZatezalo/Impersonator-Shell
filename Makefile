CC = gcc
CFLAGS = -Wall -Wextra
SRCS = doexec.c netpipe.c impersonator.c
OBJS = $(SRCS:.c=.o)
MAIN = impersonator

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) -o $(MAIN) $(OBJS) $(LIBS)
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	$(RM) *.o *.$(MAIN)