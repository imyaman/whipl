# whipl - Fast HTTP Client

A curl clone built with Perl and XS for maximum performance.

## Installation

### Quick Install

```bash
./build.sh
sudo ./install.sh
```

This installs whipl to `/usr/local/bin/whipl`.

### Custom Install Location

```bash
PREFIX=$HOME/.local ./install.sh
```

Then add `$HOME/.local/bin` to your PATH.

### Manual Build

```bash
./build.sh
./bin/whipl http://example.com
```

## Build

```bash
./build.sh
```

Or manually:
```bash
perl Makefile.PL
make
```

## Usage

```bash
# Simple GET
./bin/whipl http://example.com

# HTTPS
./bin/whipl https://example.com

# POST with data
./bin/whipl -X POST -d 'key=value' http://httpbin.org/post

# HTTPS POST
./bin/whipl -X POST -d 'secure=data' https://httpbin.org/post

# Custom headers
./bin/whipl -H 'User-Agent: whipl' -H 'Accept: application/json' http://api.example.com

# Basic authentication
./bin/whipl -u 'user:password' http://example.com/protected

# HTTPS with auth
./bin/whipl -u 'user:password' https://api.example.com/secure

# Follow redirects
./bin/whipl -L http://example.com/redirect

# Verbose mode
./bin/whipl -v https://example.com

# Save to file
./bin/whipl -o output.html http://example.com

# Headers only
./bin/whipl -I http://example.com

# Include headers in output
./bin/whipl --include http://example.com
```

## Features

- HTTP/1.1 and HTTPS requests (GET, POST, PUT, DELETE, etc.)
- TLS 1.2/1.3 support with certificate verification
- Custom headers (-H)
- Request body (-d)
- Basic authentication (-u)
- Follow redirects (-L)
- Verbose mode (-v)
- Timeout support
- Chunked transfer encoding
- Fast C implementation with Perl bindings
- Zero-copy buffer management
- Save to file (-o)
- Headers only (-I)
- Include headers in output (--include)

## Status

Phase 2 complete: Essential HTTP features (methods, headers, body, basic auth).
Phase 3 optimizations: Inline functions, optimized buffer growth, compiler flags.
Phase 4 complete: HTTPS/TLS support with OpenSSL, certificate verification.
Phase 5 in progress: Redirect following, timeout support, verbose mode.
Phase 6 in progress: Output options (save to file, headers only, include headers).
