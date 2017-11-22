# define version: Qt, gtk
VARIANT?=gtk
# debug ?
DEBUG?=0
# compiler
CC?=gcc

EXECUTABLE_QT=qmpdisplay
EXECUTABLE_GTK=gmpdisplay

# source files
SOURCES= \
	options.c \
	mpd.c \
	mpd_status.c

ifeq ($(VARIANT),gtk)
SOURCES+= \
	gtkmain.c \
	gwin_disp.c
endif

ifeq ($(VARIANT),Qt)
SOURCES+= \
	qtmain.cpp \
	qwin_disp.cpp

QTW_HEADERS= \
	qwin_disp.h
endif

############################################################################
# libs to include, executable, linker
LIBS=glib-2.0 libmpdclient

ifeq ($(CC),gcc)
CXX=g++
endif
ifeq ($(CC),clang)
CXX=clang++
endif

LD=$(CC)
ifeq ($(VARIANT),Qt)
LIBS+=Qt5Widgets
LD=$(CXX)
endif
ifeq ($(VARIANT),gtk)
LIBS+=gtk+-3.0
endif

# executable name
ifeq ($(VARIANT),Qt)
EXECUTABLE=$(EXECUTABLE_QT)
endif
ifeq ($(VARIANT),gtk)
EXECUTABLE=$(EXECUTABLE_GTK)
endif

# flags
ifeq ($(DEBUG),0)
OPTFLAGS+=-O2
else
OPTFLAGS+=-ggdb
CPPFLAGS+=-DDEBUG_NOMPD
endif

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
	rm -f $(OBJECTS) $(DEPS)
	rm -f $(EXECUTABLE_GTK) $(EXECUTABLE_QT)
	rm -rf $(OBJDIR) $(MOCDIR)
	rm -f *.moc.cpp

run: $(EXECUTABLE)
	./$(EXECUTABLE)

memcheck: all
	valgrind --leak-check=full ./$(EXECUTABLE)
slicecheck: all
	G_SLICE=debug-blocks ./$(EXECUTABLE)
