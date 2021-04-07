SRCS = str_list.c command.c wish.c
OBJS = $(SRCS:.c=.o)
TARG = wish

$(TARG): $(OBJS)
	gcc -o $(TARG) $(OBJS)

%.o: %.c
	gcc -c $< -o $@

clean:
	rm $(TARG) $(OBJS)
