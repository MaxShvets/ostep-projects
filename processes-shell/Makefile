SRCS = str_list.c command.c wish.c
OBJS = $(SRCS:.c=.o)
TARG = wish

ifdef DEBUG
OPTS = -g
else
OPTS =
endif

$(TARG): $(OBJS)
	gcc $(OPTS) -o $(TARG) $(OBJS)

%.o: %.c
	gcc $(OPTS) -c $< -o $@

clean:
	rm $(TARG) $(OBJS)
