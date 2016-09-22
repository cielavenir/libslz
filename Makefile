TOPDIR     := $(PWD)
DESTDIR    :=
PREFIX     := /usr/local
LIBDIR     := lib

CROSS_COMPILE :=

CC         := $(CROSS_COMPILE)gcc
OPT_CFLAGS := -O3
CPU_CFLAGS := -fomit-frame-pointer
DEB_CFLAGS := -Wall -g
DEF_CFLAGS :=
USR_CFLAGS :=
LIB_CFLAGS := -fPIC
INC_CFLAGS := -I$(TOPDIR)/include
CFLAGS     := $(OPT_CFLAGS) $(CPU_CFLAGS) $(DEB_CFLAGS) $(DEF_CFLAGS) $(USR_CFLAGS) $(INC_CFLAGS)

LD         := $(CC)
DEB_LFLAGS := -g
USR_LFLAGS :=
LIB_LFLAGS :=
LDFLAGS    := $(DEB_LFLAGS) $(USR_LFLAGS) $(LIB_LFLAGS)

AR         := $(CROSS_COMPILE)ar
STRIP      := $(CROSS_COMPILE)strip
BINS       := zdec zenc
STATIC     := libslz.a
SHARED     := libslz.so
SONAME     := $(SHARED).1
OBJS       :=
OBJS       += $(patsubst %.c,%.o,$(wildcard src/*.c))
OBJS       += $(patsubst %.S,%.o,$(wildcard src/*.S))

all: static shared tools

static: $(STATIC)

shared: $(SHARED)

tools: $(BINS)

zdec: src/zdec.o
	$(LD) $(LDFLAGS) -o $@ $^

zenc: src/zenc.o src/slz.o
	$(LD) $(LDFLAGS) -o $@ $^

$(STATIC): src/slz.o
	$(AR) rv $@ $^

$(SONAME): src/slz-pic.o
	$(LD) -shared $(LDFLAGS) -Wl,-soname,$@ -o $@ $^

$(SHARED): $(SONAME)
	ln -sf $^ $@

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

%-pic.o: %.c
	$(CC) $(CFLAGS) $(LIB_CFLAGS) -o $@ -c $<

install: install-headers install-static install-shared install-tools

install-headers:
	[ -d "$(DESTDIR)$(PREFIX)/include/." ] || mkdir -p -m 0755 $(DESTDIR)$(PREFIX)/include
	cp src/slz.h $(DESTDIR)$(PREFIX)/include/slz.h
	chmod 644 $(DESTDIR)$(PREFIX)/include/slz.h

install-static: static
	[ -d "$(DESTDIR)$(PREFIX)/$(LIBDIR)/." ] || mkdir -p -m 0755 $(DESTDIR)$(PREFIX)/$(LIBDIR)
	cp $(STATIC) $(DESTDIR)$(PREFIX)/$(LIBDIR)/$(STATIC)
	chmod 644 $(DESTDIR)$(PREFIX)/$(LIBDIR)/$(STATIC)

install-shared: shared
	[ -d "$(DESTDIR)$(PREFIX)/$(LIBDIR)/." ] || mkdir -p -m 0755 $(DESTDIR)$(PREFIX)/$(LIBDIR)
	cp    $(SONAME) $(DESTDIR)$(PREFIX)/$(LIBDIR)/$(SONAME)
	cp -P $(SHARED) $(DESTDIR)$(PREFIX)/$(LIBDIR)/$(SHARED)
	chmod 644 $(DESTDIR)$(PREFIX)/$(LIBDIR)/$(SONAME)

install-tools: tools
	$(STRIP) zenc
	[ -d "$(DESTDIR)$(PREFIX)/bin/." ] || mkdir -p -m 0755 $(DESTDIR)$(PREFIX)/bin
	cp zdec $(DESTDIR)$(PREFIX)/bin/zdec
	cp zenc $(DESTDIR)$(PREFIX)/bin/zenc
	chmod 755 $(DESTDIR)$(PREFIX)/bin/zdec
	chmod 755 $(DESTDIR)$(PREFIX)/bin/zenc

clean:
	-rm -f $(BINS) $(OBJS) $(STATIC) $(SHARED) *.[oa] *.so *.so.* *~ */*.[oa] */*~
