LIBNAME=template
LIBVER=0.0.1

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

PROGRAMS :=
PROGRAMS += templatectl
PROGRAMS += test

INST_FLAGS = -D

INST_PROGRAMS :=
INST_PROGRAMS += templatectl

INST_MAN1 :=
INST_MAN1 += templatectl.1

INST_MAN3 :=
INST_MAN3 += libtemplate.3

INST_HEADERS :=
INST_HEADERS += template.h

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
LDFLAGS += -syslibroot $(MACSDK)
INST_FLAGS =
INST_DIRS = $(bindir) $(man1dir) $(man3dir) $(incdir) $(libdir)
install: $(INST_DIRS)
$(INST_DIRS):
	$(Q)install -d $@
endif

# HAVE_DEP := $(shell $(PKG_CONFIG) --exists dep-1.0 2>/dev/null && echo 'yes')
# ifeq ($(HAVE_DEP),yes)
# EXTRA_OBJS += extra.o
# PROGRAMS += progdep
# INST_MAN1 += progdep.1
# progdep-cflags := $(shell $(PKG_CONFIG) --cflags dep-1.0)
# progdep-ldflags :=
# progdep-ldlibs := $(shell $(PKG_CONFIG) --libs dep-1.0)
# else
# $(warning Your system does not have dep, skipping progdep)
# endif

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
	@echo '#define TEMPLATE_VERSION "$(VERSION)"' > version.h.tmp
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
