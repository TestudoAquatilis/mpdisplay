CC=gcc
CXX=g++
LD=$(CC)

# define version: Qt, gtk
VERSION=Qt

# libs to include
LIBS=glib-2.0 libmpdclient
ifeq ($(VERSION),Qt)
LIBS+=Qt5Widgets
LD=$(CXX)
endif
ifeq ($(VERSION),gtk)
LIBS+=gtk+-3.0
endif

# source files
SOURCES= \
	options.c \
	mpd.c \
	mpd_status.c

ifeq ($(VERSION),gtk)
SOURCES+= \
	gtkmain.c \
	disp_window.c
endif

ifeq ($(VERSION),Qt)
SOURCES+= \
	qtmain.cpp \
	win_disp.cpp

QTW_HEADERS= \
	win_disp.h
endif

# executable name
EXECUTABLE=mpdisplay

# flags
OPTFLAGS=-O2
#OPTFLAGS=-ggdb
#CPPFLAGS+=-DDEBUG_NOMPD

############################################################################
# flags and co
COMPILEFLAGS=-c -Wall -fpic -I.
CFLAGS=$(COMPILEFLAGS) -std=gnu99 $(OPTFLAGS) $(CPPFLAGS)
CXXFLAGS=$(COMPILEFLAGS) -std=c++11 $(OPTFLAGS) $(CPPFLAGS)
LDFLAGS=$(OPTFLAGS)

CFLAGS+=$(shell pkg-config --cflags $(LIBS))
CXXFLAGS+=$(shell pkg-config --cflags $(LIBS))
LDFLAGS+=$(shell pkg-config --libs $(LIBS))

OBJDIR=obj
MOCDIR=moc

CSOURCES=$(filter %.c,$(SOURCES))
CXXSOURCES=$(filter %.cpp,$(SOURCES))
MOCSOURCES=$(QTW_HEADERS:%.h=$(MOCDIR)/%.moc.cpp)
OBJECTS=$(CSOURCES:%.c=$(OBJDIR)/%.o)
OBJECTS+=$(CXXSOURCES:%.cpp=$(OBJDIR)/%.o)
OBJECTS+=$(MOCSOURCES:$(MOCDIR)/%.cpp=$(OBJDIR)/%.o)
DEPS=$(CSOURCES:%.c=$(OBJDIR)/%.d)
DEPS+=$(CXXSOURCES:%.cpp=$(OBJDIR)/%.d)

############################################################################
# rules
.PHONY: all
all: $(EXECUTABLE)
.SECONDARY: $(MOCSOURCES)

-include $(OBJECTS:.o=.d)

$(EXECUTABLE): $(OBJECTS) Makefile
	$(LD) $(LDFLAGS) $(OBJECTS) -o $@

$(OBJDIR)/%.o: %.c Makefile | $(OBJDIR)
	$(CC) -MM $(CFLAGS) $*.c > $(OBJDIR)/$*.d
	sed -i -e "s/\\(.*\\.o:\\)/$(OBJDIR)\\/\\1/" $(OBJDIR)/$*.d
	$(CC) $(CFLAGS) $*.c -o $(OBJDIR)/$*.o

$(MOCDIR)/%.moc.cpp: %.h Makefile | $(MOCDIR)
	moc $< > $@

$(OBJDIR)/%.o: %.cpp Makefile | $(OBJDIR)
	$(CXX) -MM $(CXXFLAGS) $*.cpp > $(OBJDIR)/$*.d
	sed -i -e "s/\\(.*\\.o:\\)/$(OBJDIR)\\/\\1/" $(OBJDIR)/$*.d
	$(CXX) $(CXXFLAGS) $*.cpp -o $(OBJDIR)/$*.o

$(OBJDIR)/%.o: $(MOCDIR)/%.cpp Makefile | $(OBJDIR)
	$(CXX) -MM $(CXXFLAGS) $(MOCDIR)/$*.cpp > $(OBJDIR)/$*.d
	sed -i -e "s/\\(.*\\.o:\\)/$(OBJDIR)\\/\\1/" $(OBJDIR)/$*.d
	$(CXX) $(CXXFLAGS) $(MOCDIR)/$*.cpp -o $(OBJDIR)/$*.o

$(OBJDIR) $(MOCDIR):
	mkdir -p $@

clean:
	rm -f $(EXECUTABLE) $(OBJECTS) $(DEPS)
	rm -rf $(OBJDIR) $(MOCDIR)
	rm -f *.moc.cpp

memcheck: all
	valgrind --leak-check=full ./$(EXECUTABLE)
slicecheck: all
	G_SLICE=debug-blocks ./$(EXECUTABLE)
