SRCS                   := $(wildcard *.gresource.xml)
XMLOBJS                := $(patsubst %.gresource.xml,out/%.c,$(SRCS))
OBJS                   := $(patsubst %.c,%.o,$(XMLOBJS))
ROOTDIR                := ../../../
INCS                   := $(shell pkg-config --cflags-only-I $(ROOTDIR)/gtk4.pc)

GLIB_CC_RESOURCE       := glib-compile-resources

CFLAGS_EXT             := $(shell pkg-config --cflags-only-other $(ROOTDIR)/gtk4.pc) $(INCS)
GLIB_CC_FLAGS          := --generate-source --compiler=$(CC)

all: init $(OBJS)

init:
	@if [ ! -d out ]; then \
		echo "MKDIR out"; \
		mkdir -p out; \
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
