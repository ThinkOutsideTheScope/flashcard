SRCDIR       := src
SRCS         := $(wildcard $(SRCDIR)/*/*) $(wildcard $(SRCDIR)/frontend/xml/*)
CXXSRCS      := $(filter %.cpp,$(SRCS))
CSRCS        := $(filter %.c,$(SRCS))
OBJS         := $(CXXSRCS:.cpp=.o) $(CSRCS:.c=.o)
CC           := gcc
CXX          := g++
LD           := ld
XML_OBJS      = $$(wildcard $(SRCDIR)/frontend/xml/out/*)
DYNTYPE      := dll
SUBSYSTEM    := -Wl,--subsystem 2
LIBS         := $(shell pkg-config --libs gtk4.pc)
INCS         := $(shell pkg-config --cflags-only-I gtk4.pc) -Iinc
TARGET       := out/flashcard
IS_UNIX      := no
LD_EXPORT_DYNAMIC :=
HAS_EXPORT_DYNAMIC := $(shell \
    printf 'int main(){}' | \
    $(CC) -Wl,--export-dynamic -x c - -o /dev/null >/dev/null 2>&1 \
    && echo yes)

ifeq ($(HAS_EXPORT_DYNAMIC),yes)
	LD_EXPORT_DYNAMIC := -Wl,--export-dynamic
	SUBSYSTEM := 
	IS_UNIX   := yes
endif
CPPFLAGS_EXT := $(INCS) $(shell pkg-config --cflags-only-other gtk4.pc) -std=c++17
CFLAGS_EXT   := $(INCS) $(shell pkg-config --cflags-only-other gtk4.pc) -std=c17
LDFLAGS_EXT  := $(LIBS) $(LD_EXPORT_DYNAMIC) $(SUBSYSTEM)

all: maketarget $(TARGET)
$(TARGET): $(OBJS)
	@echo "MKDIR out"
	@mkdir -p out
	$(eval XML_OBJS = $(XML_OBJS))
	@echo "CXX $^ $(XML_OBJS) -> $@"
	@$(CXX) $^ -o $@ $(XML_OBJS) $(LDFLAGS) $(LDFLAGS_EXT)
%.o: %.cpp
	@echo "CXX $< -> $@"
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CPPFLAGS_EXT) -c $< -o $@
%.o: %.c
	@echo "CC $< -> $@"
	@$(CC) $(CFLAGS) $(CFLAGS_EXT) -c $< -o $@
maketarget:
	@echo "MAKE $(SRCDIR)/frontend/xml"
	@$(MAKE) -C $(SRCDIR)/frontend/xml ROOTDIR=$(abspath .) CC=$(CC) CXX=$(CXX) LD=$(LD)

clean:
	@echo "RM $(OBJS) out"
	@rm -rf $(OBJS) out
	@echo "MAKE $(SRCDIR)/frontend/xml clean"
	@$(MAKE) -C $(SRCDIR)/frontend/xml clean
