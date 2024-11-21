SRCF := ./src
INCF := ./inc
OUTF := ./out

CC := gcc -I $(INCF)
SRCS := $(wildcard $(SRCF)/*.c)
OBJS := $(SRCS:$(SRCF)/%.c=$(OUTF)/%.o)

wr : $(OBJS)
	gcc $^ -o $@

$(OUTF)/%.o : $(SRCF)/%.c 
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTF)/%.d : $(SRCF)/%.c
	$(CC) -MM $< > $@

include $(SRCS:$(SRCF)/%.c=$(OUTF)/%.d)

clean :
	rm $(OUTF)/*