CC   = gcc
MAKE = make

UNAME_S := $(shell uname -s 2>/dev/null)
ifeq ($(OS),Windows_NT)
	DETECTED_OS := windows
else ifneq (,$(filter MINGW% MSYS% CYGWIN%,$(UNAME_S)))
	DETECTED_OS := windows
else ifeq ($(UNAME_S),Linux)
	DETECTED_OS := linux
else ifeq ($(UNAME_S),Darwin)
	DETECTED_OS := macos
else
	$(error Unable to detect host OS for IDO recomp (uname=$(UNAME_S)))
endif

RECOMP_DIR := ido-recomp/$(DETECTED_OS)
# v1.2 publishes windows builds; linux/macos stay on the pinned v1.0 archive.
ifeq ($(DETECTED_OS),windows)
	IDO_RECOMP_VERSION := v1.2
else
	IDO_RECOMP_VERSION := v1.0
endif

default: all

all: recomp

clean:
	$(RM) -rf $(RECOMP_DIR)

distclean: clean

recomp:
	@echo "Fetching IDO 5.3 recomp ($(DETECTED_OS))..."
	wget https://github.com/decompals/ido-static-recomp/releases/download/$(IDO_RECOMP_VERSION)/ido-5.3-recomp-${DETECTED_OS}.tar.gz
	mkdir -p $(RECOMP_DIR)
	tar xf ido-5.3-recomp-${DETECTED_OS}.tar.gz -C $(RECOMP_DIR)
	$(RM) ido-5.3-recomp-${DETECTED_OS}.tar.gz
ifeq ($(DETECTED_OS),windows)
	# Windows release ships err.english.cc with CRLF; IDO needs LF offsets.
	@python -c "from pathlib import Path; p=Path('$(RECOMP_DIR)/err.english.cc'); p.write_bytes(p.read_bytes().replace(b'\r\n', b'\n'))"
endif

.PHONY: all clean distclean default recomp
