LIBNAME=microui
LIBVER=2.0.1

CFLAGS ?=
LDFLAGS ?=
LDLIBS ?=

DESTDIR ?=
PREFIX ?= $(HOME)
BINDIR ?= $(PREFIX)/bin
MANDIR ?= $(PREFIX)/share/man
INCDIR ?= $(PREFIX)/include
LIBDIR ?= $(PREFIX)/lib

PKG_CONFIG ?= pkg-config

local_config ?= local.mk
-include ${local_config}

cflags = -fno-strict-aliasing
cflags += -Wall -Wwrite-strings

LIB_OBJS :=
LIB_OBJS += version.o
LIB_OBJS += microui.o

PROGRAMS :=
PROGRAMS += mudemo
PROGRAMS += test

INST_FLAGS = -D

INST_PROGRAMS :=
INST_PROGRAMS += mudemo

INST_MAN1 :=
INST_MAN1 += mudemo.1

INST_MAN3 :=
INST_MAN3 += libmicroui.3

INST_HEADERS :=
INST_HEADERS += microui.h

INST_LIBS :=
INST_LIBS += lib$(LIBNAME).a

bindir := $(DESTDIR)$(BINDIR)
man1dir := $(DESTDIR)$(MANDIR)/man1
man3dir := $(DESTDIR)$(MANDIR)/man3
incdir := $(DESTDIR)$(INCDIR)
libdir := $(DESTDIR)$(LIBDIR)

# Pretty print
V := @
Q := $(V:1=)

all:

OSNAME := $(shell uname)
ifeq ($(OSNAME),Darwin)
#MACSDK = /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk
MACSDK = $(shell xcodebuild -version -sdk macosx Path)
LDFLAGS += -lSystem
LDFLAGS += -framework OpenGL
LDFLAGS += -syslibroot $(MACSDK)
INST_FLAGS =
INST_DIRS = $(bindir) $(man1dir) $(man3dir) $(incdir) $(libdir)
install: $(INST_DIRS)
$(INST_DIRS):
	$(Q)install -d $@
endif

HAVE_SDL := $(shell $(PKG_CONFIG) --exists sdl2 2>/dev/null && echo 'yes')
ifeq ($(HAVE_SDL),yes)
LIB_OBJS += sdl.o
sdl-cflags := $(shell $(PKG_CONFIG) --cflags sdl2)
sdl-ldflags :=
sdl-ldlibs :=
CFLAGS += -DUSING_SDL
LDLIBS := $(shell $(PKG_CONFIG) --libs sdl2)
else
$(error SDL2 is required)
endif

LIBS := lib$(LIBNAME).a
OBJS := $(LIB_OBJS) $(EXTRA_OBJS) $(PROGRAMS:%=%.o)

CPPFLAGS := -Iinclude

all: $(PROGRAMS)

ldflags += $($(@)-ldflags) $(LDFLAGS)
ldlibs  += $($(@)-ldlibs)  $(LDLIBS)
$(PROGRAMS): % : %.o $(LIBS)
	@echo "  LD      $@"
	$(Q)$(LD) $(ldflags) $^ $(ldlibs) -o $@

lib$(LIBNAME).a: $(LIB_OBJS)
	@echo "  AR      $@"
	$(Q)$(AR) rcs $@ $^


cflags   += $($(*)-cflags) $(CPPFLAGS) $(CFLAGS)
%.o: %.c
	@echo "  CC      $@"
	$(Q)$(CC) $(cflags) -c -o $@ $<

VERSION:=$(shell git describe --dirty 2>/dev/null || echo '$(LIBVER)')
version.o: version.h
version.h: FORCE
	@echo '#define MICROUI_VERSION "$(VERSION)"' > version.h.tmp
	@if cmp -s version.h version.h.tmp; then \
		rm version.h.tmp; \
	else \
		echo "  GEN     $@"; \
		mv version.h.tmp version.h; \
	fi

clean:
	@echo "  CLEAN"
	@rm -f *.[oa] .*.d $(PROGRAMS) version.h

install: install-bin install-man install-inc install-lib
install-bin: $(INST_PROGRAMS:%=$(bindir)/%)
install-man: $(INST_MAN1:%=$(man1dir)/%)
install-man: $(INST_MAN3:%=$(man3dir)/%)
install-inc: $(INST_HEADERS:%=$(incdir)/%)
install-lib: $(INST_LIBS:%=$(libdir)/%)

$(bindir)/%: %
	@echo "  INSTALL $@"
	$(Q)install $(INST_FLAGS) $< $@ || exit 1;
$(man1dir)/%: man1/%
	@echo "  INSTALL $@"
	$(Q)install $(INST_FLAGS) -m 644 $< $@ || exit 1;
$(man3dir)/%: man3/%
	@echo "  INSTALL $@"
	$(Q)install $(INST_FLAGS) -m 644 $< $@ || exit 1;
$(incdir)/%: include/%
	@echo "  INSTALL $@"
	$(Q)install $(INST_FLAGS) -m 644 $< $@ || exit 1;
$(libdir)/%: %
	@echo "  INSTALL $@"
	$(Q)install $(INST_FLAGS) $< $@ || exit 1;

.PHONY: FORCE

# GCC's dependencies
-include $(OBJS:%.o=.%.o.d)
