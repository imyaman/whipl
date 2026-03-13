#!/bin/bash
set -e

echo "Building whipl..."

# Create blib directories
mkdir -p blib/arch/auto/Whipl blib/lib

# Copy Perl module
cp lib/Whipl.pm blib/lib/

# Generate XS C code
/usr/bin/perl /usr/share/perl5/vendor_perl/ExtUtils/xsubpp \
    -typemap '/usr/share/perl5/ExtUtils/typemap' \
    xs/whipl.xs > xs/whipl.c

# Compile C files with optimizations
CFLAGS="-I. -O3 -march=native -fPIC -I/usr/lib64/perl5/CORE -D_REENTRANT -D_GNU_SOURCE -finline-functions"

gcc -c $CFLAGS xs/whipl.c -o Whipl.o
gcc -c $CFLAGS src/http.c -o http.o
gcc -c $CFLAGS src/transport.c -o transport.o
gcc -c $CFLAGS src/buffer.c -o buffer.o

# Link shared library
gcc -shared Whipl.o http.o transport.o buffer.o \
    -o blib/arch/auto/Whipl/Whipl.so -lperl -lssl -lcrypto

echo "Build complete!"
