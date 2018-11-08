#
# Copyright 2018 Red Hat, Inc.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

NAME    = timer

SOURCES = ${NAME}.c

ARCH    = x86_64

CC      = gcc

LD      = ld

OC      = objcopy

CFLAGS  = -c                          \
          -fno-stack-protector        \
          -fpic                       \
          -fshort-wchar               \
          -I /usr/include/efi         \
          -I /usr/include/efi/${ARCH} \
          -mno-red-zone               \
          -pedantic                   \
          -std=c18                    \
          -Wall                       \
          -Werror                     \
          -Wextra

LDFLAGS = --library-path=/usr/lib64            \
          --library=:gnuefi/crt0-efi-${ARCH}.o \
          --library=:libefi.a                  \
          --library=:libgnuefi.a               \
          --script=gnuefi/elf_${ARCH}_efi.lds  \
          -Bshareable                          \
          -Bsymbolic                           \
          -nostdlib                            \
          -z nocombreloc

OCFLAGS = --only-section=.data     \
          --only-section=.dynamic  \
          --only-section=.dynsym   \
          --only-section=.rel      \
          --only-section=.rela     \
          --only-section=.reloc    \
          --only-section=.sdata    \
          --only-section=.text     \
          --target=efi-app-${ARCH}


# disable built-in implicit rules
MAKEFLAGS += --no-builtin-rules


# default target
PHONY = all
all: ${NAME}.efi


# object files
%.o: %.c
	${CC} ${CFLAGS} $< -o $@


# shared library
${NAME}.so: ${patsubst %.c,%.o,${SOURCES}}
	${LD} $^ ${LDFLAGS} --output=$@


# EFI binary
${NAME}.efi: ${NAME}.so
	${OC} ${OCFLAGS} $< $@


PHONY += clean
clean:
	rm --force ${patsubst %.c,%.o,${SOURCES}} ${NAME}.so ${NAME}.efi


.PHONY: ${PHONY}
