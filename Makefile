SRCDIR       := src
SRCS         := $(wildcard $(SRCDIR)/*/*) $(wildcard $(SRCDIR)/frontend/xml/*)
CXXSRCS      := $(filter %.cpp,$(SRCS))
CSRCS        := $(filter %.c,$(SRCS))
OBJS         := $(CXXSRCS:.cpp=.o) $(CSRCS:.c=.o)
XML_OBJS      = $$(wildcard $(SRCDIR)/frontend/xml/out/*)
DEPPREFIX    := ucrt64
DYNTYPE      := dll
SUBSYSTEM    := -Wl,--subsystem 2
LIBS         := -L$(DEPPREFIX)/lib -lgtk-4 -lpangocairo-1.0 -lpangowin32-1.0 -lpango-1.0 -lgdk_pixbuf-2.0 -lcairo-gobject -lcairo -lharfbuzz -lvulkan-1 -lgraphene-1.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0 -lintl -ladwaita-1
INCS         := -I$(DEPPREFIX)/include/gtk-4.0 -I$(DEPPREFIX)/include/pango-1.0 -I$(DEPPREFIX)/include/fribidi -I$(DEPPREFIX)/include -I$(DEPPREFIX)/include -I$(DEPPREFIX)/include/harfbuzz \
	-I$(DEPPREFIX)/include/gdk-pixbuf-2.0 -I$(DEPPREFIX)/include/cairo -I$(DEPPREFIX)/include/freetype2 -I$(DEPPREFIX)/include/libpng16 -I$(DEPPREFIX)/include/pixman-1 \
	-I$(DEPPREFIX)/include/graphene-1.0 -I$(DEPPREFIX)/lib/graphene-1.0/include -I$(DEPPREFIX)/include/glib-2.0 -I$(DEPPREFIX)/lib/glib-2.0/include -I$(DEPPREFIX)/include \
	-Iinc
TARGET       := out/flashcard.exe
CPPFLAGS_EXT := $(INCS) -mfpmath=sse -msse -msse2 -mms-bitfields -std=c++17
CFLAGS_EXT   := $(INCS) -mfpmath=sse -msse -msse2 -mms-bitfields -std=c17
LDFLAGS_EXT  := $(LIBS) -Wl,--no-as-needed $(SUBSYSTEM)

GLIB_XML     := glib-compile-resources

all: maketarget $(TARGET)
$(TARGET): $(OBJS)
	@if [ ! -d out ]; then \
		echo "MKDIR out"; \
		mkdir out; \
		for dll in $$(find $(DEPPREFIX)/bin -name '*.$(DYNTYPE)' | tr '\n' ' ' | sed 's|$(DEPPREFIX)/bin/||g'); do \
			echo "CP $(DEPPREFIX)/bin/$$dll -> out/$$dll"; \
			cp $(DEPPREFIX)/bin/$$dll out/$$dll; \
		done; \
		echo "MKDIR out/share/gtk-4.0"; \
		mkdir -p out/share/gtk-4.0; \
		echo "CP $(DEPPREFIX)/share/gtk-4.0 -> out/share/gtk-4.0"; \
		cp -r $(DEPPREFIX)/share/gtk-4.0/* out/share/gtk-4.0; \
		echo "MKDIR out/share/glib-2.0"; \
		mkdir -p out/share/glib-2.0/schemas; \
		echo "CP $(DEPPREFIX)/share/glib-2.0/schemas -> out/share/glib-2.0/schemas"; \
		cp -r $(DEPPREFIX)/share/glib-2.0/schemas/* out/share/glib-2.0/schemas; \
		echo "MKDIR out/share/icons"; \
		mkdir -p out/share/icons; \
		echo "CP $(DEPPREFIX)/share/icons -> out/share/icons"; \
		cp -r $(DEPPREFIX)/share/icons/* out/share/icons; \
		echo "MKDIR out/gdk-pixbuf-2.0"; \
		mkdir out/share/gdk-pixbuf-2.0; \
		echo "CP $(DEPPREFIX)/lib/gdk-pixbuf-2.0 -> out/share/gdk-pixbuf-2.0"; \
		cp -r $(DEPPREFIX)/lib/gdk-pixbuf-2.0/* out/share/gdk-pixbuf-2.0; \
		echo "CP $(DEPPREFIX)/bin/libadwaita-1-0.dll -> out/libadwaita-1-0.dll"; \
		cp $(DEPPREFIX)/bin/libadwaita-1-0.dll out/libadwaita-1-0.dll; \
	fi;
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
	@$(MAKE) -C $(SRCDIR)/frontend/xml

clean:
	@echo "RM $(OBJS) out"
	@rm -rf $(OBJS) out
	@echo "MAKE $(SRCDIR)/frontend/xml clean"
	@$(MAKE) -C $(SRCDIR)/frontend/xml clean
XZ_FLAGS      := -e -9
DELETE_UCRT64 := false
# compress_ucrt64 is mostly an internal helper and usually won't be used
compress_ucrt64:
	@echo "TAR ucrt64 -> ucrt64.tar.xz"
	@tar -c -I "xz $(XZ_FLAGS)" -f ucrt64.tar.xz ucrt64
	@if [ "$(DELETE_UCRT64)" == "true" ]; then \
		echo "RM ucrt64"; \
		rm -rf ucrt64; \
	fi;
decompress_ucrt64:
	@echo "TAR ucrt64.tar.xz -> ucrt64"
	@tar -xJf ucrt64.tar.xz
	@if [ "$(DELETE_UCRT64)" == "true" ]; then \
		echo "RM ucrt64.tar.xz"; \
		rm ucrt64.tar.xz; \
	fi;