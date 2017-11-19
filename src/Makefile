CC=gcc
CXX=g++
OPTFLAGS=-O2
#OPTFLAGS=-ggdb
#CPPFLAGS+=-DDEBUG_NOMPD
COMPILEFLAGS=-c -Wall -fpic
CFLAGS=$(COMPILEFLAGS) -std=gnu99 $(OPTFLAGS) $(CPPFLAGS)
CXXFLAGS=$(COMPILEFLAGS) $(OPTFLAGS) $(CPPFLAGS)
LDFLAGS=$(OPTFLAGS)

LIBS=Qt5Widgets glib-2.0 libmpdclient
#LIBS=gtk+-3.0 glib-2.0 libmpdclient

CFLAGS+=$(shell pkg-config --cflags $(LIBS))
CXXFLAGS+=$(shell pkg-config --cflags $(LIBS))
LDFLAGS+=$(shell pkg-config --libs $(LIBS))

SOURCES= \
	options.c \
	mpd.c \
	mpd_status.c

#SOURCES+= \
#	main.c \
#	disp_window.c

SOURCES+= \
	qtmain.cpp \
	win_disp.cpp

QTW_HEADERS= \
	win_disp.h

EXECUTABLE=mpdisplay

OBJDIR=obj

SOURCES+=$(QTW_HEADERS:%.h=%.moc.cpp)
CSOURCES=$(filter %.c,$(SOURCES))
CXXSOURCES=$(filter %.cpp,$(SOURCES))
OBJECTS=$(CSOURCES:%.c=$(OBJDIR)/%.o)
OBJECTS+=$(CXXSOURCES:%.cpp=$(OBJDIR)/%.o)
DEPS=$(CSOURCES:%.c=$(OBJDIR)/%.d)
DEPS+=$(CXXSOURCES:%.cpp=$(OBJDIR)/%.d)

.PHONY: all
all: $(EXECUTABLE)

-include $(OBJECTS:.o=.d)

$(EXECUTABLE): $(OBJECTS) Makefile
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@

$(OBJDIR)/%.o: %.c Makefile | $(OBJDIR)
	$(CC) -MM $(CFLAGS) $*.c > $(OBJDIR)/$*.d
	sed -i -e "s/\\(.*\\.o:\\)/$(OBJDIR)\\/\\1/" $(OBJDIR)/$*.d
	$(CC) $(CFLAGS) $*.c -o $(OBJDIR)/$*.o

%.moc.cpp: %.h Makefile
	moc $< > $@

$(OBJDIR)/%.o: %.cpp Makefile | $(OBJDIR)
	$(CXX) -MM $(CXXFLAGS) $*.cpp > $(OBJDIR)/$*.d
	sed -i -e "s/\\(.*\\.o:\\)/$(OBJDIR)\\/\\1/" $(OBJDIR)/$*.d
	$(CC) $(CXXFLAGS) $*.cpp -o $(OBJDIR)/$*.o

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -f $(EXECUTABLE) $(OBJECTS) $(DEPS)
	rm -rf $(OBJDIR)
	rm -f *.moc.cpp

memcheck: all
	valgrind --leak-check=full ./$(EXECUTABLE)
slicecheck: all
	G_SLICE=debug-blocks ./$(EXECUTABLE)
