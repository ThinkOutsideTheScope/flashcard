SRCS                   := $(wildcard *.gresource.xml)
XMLOBJS                := $(patsubst %.gresource.xml,out/%.c,$(SRCS))
OBJS                   := $(patsubst %.c,%.o,$(XMLOBJS))
DEPPREFIX              := ucrt64
INCS                   := -I$(DEPPREFIX)/include/gtk-4.0 -I$(DEPPREFIX)/include/pango-1.0 -I$(DEPPREFIX)/include/fribidi -I$(DEPPREFIX)/include -I$(DEPPREFIX)/include -I$(DEPPREFIX)/include/harfbuzz \
	-I$(DEPPREFIX)/include/gdk-pixbuf-2.0 -I$(DEPPREFIX)/include/cairo -I$(DEPPREFIX)/include/freetype2 -I$(DEPPREFIX)/include/libpng16 -I$(DEPPREFIX)/include/pixman-1 \
	-I$(DEPPREFIX)/include/graphene-1.0 -I$(DEPPREFIX)/lib/graphene-1.0/include -I$(DEPPREFIX)/include/glib-2.0 -I$(DEPPREFIX)/lib/glib-2.0/include

GLIB_CC_RESOURCE       := glib-compile-resources

CFLAGS_EXT             := -mfpmath=sse -msse -msse2 -mms-bitfields $(INCS)
GLIB_CC_FLAGS          := --generate-source --compiler=$(CC)

all: init $(OBJS)

init:
	@if [ ! -d out ]; then \
		echo "MKDIR out"; \
		mkdir out; \
	fi
out/%.c: %.gresource.xml
	@echo "GLIB_CC_RESOURCE $^"
	@$(GLIB_CC_RESOURCE) $(GLIB_CC_FLAGS) $^ --target=$@
out/%.o: out/%.c
	@echo "CC $^ -> $@"
	@$(CC) $(CFLAGS) $(CFLAGS_EXT) -c $^ -o $@
clean:
	@echo "RM out"
	@rm -rf out