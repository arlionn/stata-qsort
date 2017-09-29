EXECUTION=normal

ifeq ($(OS),Windows_NT)
	OSFLAGS = -shared
	GCC = x86_64-w64-mingw32-gcc-5.4.0.exe
	PLUG = build/qsort_windows.plugin
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		OSFLAGS = -shared -fPIC -DSYSTEM=OPUNIX
		PLUG = build/qsort_unix.plugin
	endif
	ifeq ($(UNAME_S),Darwin)
		OSFLAGS = -bundle -DSYSTEM=APPLEMAC
		PLUG = build/qsort_macosx.plugin
	endif
	GCC = gcc
endif

ifeq ($(EXECUTION),windows)
	OSFLAGS = -shared
	GCC = x86_64-w64-mingw32-gcc
	PLUG = build/qsort_windows.plugin
endif

SPI = 2.0
SPT = 0.2
CFLAGS = -Wall -O3 $(OSFLAGS)
AUX = build/stplugin.o
OUT = $(PLUG) build/qsort.o

# OpenMP only tested on Linux
ifeq ($(OS),Windows_NT)
all: clean links qsort_other
else ifeq ($(EXECUTION),windows)
all: clean links qsort_other
else ifeq ($(UNAME_S),Darwin)
all: clean links qsort_other
else ifeq ($(UNAME_S),Linux)
all: clean links qsort_nix
endif

links:
	rm -f  src/plugin/lib
	rm -f  src/plugin/spt
	rm -f  src/plugin/spi
	ln -sf ../../lib 	  src/plugin/lib
	ln -sf lib/spt-$(SPT) src/plugin/spt
	ln -sf lib/spi-$(SPI) src/plugin/spi

qsort_other: src/plugin/qsort_plugin.c src/plugin/spi/stplugin.c
	mkdir -p ./build
	$(GCC) $(CFLAGS) -c -o build/stplugin.o    src/plugin/spi/stplugin.c
	$(GCC) $(CFLAGS) -c -o build/qsort.o       src/plugin/qsort_plugin.c
	$(GCC) $(CFLAGS)    -o $(PLUG)  src/plugin/spi/stplugin.c src/plugin/qsort_plugin.c
	cp $(PLUG) lib/plugin/

qsort_nix: src/plugin/qsort_plugin.c src/plugin/spi/stplugin.c
	mkdir -p ./build
	$(GCC) $(CFLAGS) -c -o build/stplugin.o    src/plugin/spi/stplugin.c
	$(GCC) $(CFLAGS) -c -o build/qsort.o       src/plugin/qsort_plugin.c
	$(GCC) $(CFLAGS)    -o $(PLUG)  src/plugin/spi/stplugin.c src/plugin/qsort_plugin.c
	cp $(PLUG) lib/plugin/

.PHONY: clean
clean:
	rm -f $(OUT) $(OUTM) $(AUX)
