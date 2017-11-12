CC=gcc
#OPTFLAGS=-O2
OPTFLAGS=-ggdb
CFLAGS=-c -Wall -std=gnu99 $(OPTFLAGS)
LDFLAGS=$(OPTFLAGS)

LIBS=gtk+-3.0 glib-2.0 libmpdclient

CFLAGS+=$(shell pkg-config --cflags $(LIBS))
LDFLAGS+=$(shell pkg-config --libs $(LIBS))

SOURCES= \
	main.c \
	mpd.c \
	disp_window.c \
	mpd_status.c \
	options.c

EXECUTABLE=mpdisplay

OBJDIR=obj
OBJECTS=$(SOURCES:%.c=$(OBJDIR)/%.o)
DEPS=$(SOURCES:%.c=$(OBJDIR)/%.d)

.PHONY: all
all: $(EXECUTABLE)

-include $(OBJECTS:.o=.d)

$(EXECUTABLE): $(OBJECTS) Makefile
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

$(OBJDIR)/%.o: %.c Makefile | $(OBJDIR)
	$(CC) -MM $(CFLAGS) $*.c > $(OBJDIR)/$*.d
	sed -i -e "s/\\(.*\\.o:\\)/$(OBJDIR)\\/\\1/" $(OBJDIR)/$*.d
	$(CC) $(CFLAGS) $*.c -o $(OBJDIR)/$*.o

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -f $(EXECUTABLE) $(OBJECTS) $(DEPS)
	rm -rf $(OBJDIR)

memcheck: all
	valgrind --leak-check=full ./$(EXECUTABLE)
slicecheck: all
	G_SLICE=debug-blocks ./$(EXECUTABLE)
