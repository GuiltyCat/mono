CC=gcc

CFLAGS=-std=c17 -O3 -Wall -Wextra -pedantic -I./

LDFLAGS=-lm  # $(shell pkg-config --libs opencv)

SRC=mono_image.c mono_music.c

OBJDIR=./obj
OBJ=$(addprefix $(OBJDIR)/,$(subst .c,.o, $(subst .cc,.o, $(subst .cpp,.o, $(SRC)))))

PROJECT=main.out

.PHONY:all clean run $(PROJECT) 


all:$(OBJSDIR) $(PROJECT) 

$(PROJECT):$(OBJ) 
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJDIR):$(OBJDIR)
	mkdir -p $@

$(OBJDIR)/%.o: %.c
	$(shell if [ ! -d $(OBJDIR) ]; then mkdir -p $(OBJDIR); fi)
	$(CC) $(CFLAGS) -c -o $@ $< 

clean:
	\rm -rf $(PROJECT)
	\rm -rf $(OBJ)
	\rmdir $(OBJDIR)

run:$(PROJECT)
	./$(PROJECT)

