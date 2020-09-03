CC=gcc

CFLAGS=-std=c17 -O3 -Wall -Wextra -pedantic -I./

LDFLAGS=-lm

SRC=$(wildcard *.c)

OBJDIR=./obj
OBJ=$(addprefix $(OBJDIR)/,$(subst .c,.o, $(subst .cc,.o, $(subst .cpp,.o, $(SRC)))))
DEP=$(subst .o,.d,$(OBJ))

PROJECT=main.out

.PHONY:all clean run $(PROJECT) 


all:$(OBJSDIR) $(PROJECT) 

$(PROJECT):$(OBJ) 
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

-include $(DEP)
$(OBJDIR)/%.o:%.c $(OBJDIR)/%.d
	$(CC) $(CFLAGS) -MMD -c -o $@ $<

$(OBJDIR)/%.d:%.c
	$(CC) $< -MM -MP -MF $@

$(OBJDIR):$(OBJDIR)
	mkdir -p $@

clean:
	\rm -rf $(PROJECT)
	\rm -rf $(OBJ)

run:$(PROJECT)
	./$(PROJECT)

