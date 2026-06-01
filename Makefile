CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -g
LDFLAGS = -lncurses
TARGET  = graphics_editor
SRCDIR  = src
SRCS    = $(SRCDIR)/main.c \
          $(SRCDIR)/canvas.c \
          $(SRCDIR)/shapes.c \
          $(SRCDIR)/objects.c \
          $(SRCDIR)/menu.c
OBJS    = $(SRCS:.c=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

run: all
	./$(TARGET)

clean:
	rm -f $(SRCDIR)/*.o $(TARGET)
