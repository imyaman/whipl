#!/bin/bash
set -e

PREFIX="${PREFIX:-/usr/local}"
BINDIR="$PREFIX/bin"
LIBDIR="$PREFIX/lib/whipl"

echo "Installing whipl to $PREFIX"
echo

# Check if we need sudo
SUDO=""
if [ ! -w "$PREFIX" ]; then
    echo "Note: Installation requires root privileges"
    SUDO="sudo"
fi

# Build if needed
if [ ! -f "blib/arch/auto/Whipl/Whipl.so" ]; then
    echo "Building whipl..."
    ./build.sh
fi

# Create directories
echo "Creating directories..."
$SUDO mkdir -p "$BINDIR"
$SUDO mkdir -p "$LIBDIR/lib"
$SUDO mkdir -p "$LIBDIR/arch/auto/Whipl"

# Install Perl module
echo "Installing Perl module..."
$SUDO cp -r blib/lib/* "$LIBDIR/lib/"
$SUDO cp -r blib/arch/* "$LIBDIR/arch/"

# Install binary with updated library paths
echo "Installing binary..."
cat > /tmp/whipl << 'EOF'
#!/usr/bin/env perl
use strict;
use warnings;
use FindBin;
use lib "LIBDIR/lib";
use lib "LIBDIR/arch";
use Whipl;
use Getopt::Long;

my ($method, @headers, $data, $user, $follow, $verbose, $output, $head_only, $include_headers, $help);
GetOptions(
    'X|request=s' => \$method,
    'H|header=s@' => \@headers,
    'd|data=s'    => \$data,
    'u|user=s'    => \$user,
    'L|location'  => \$follow,
    'v|verbose'   => \$verbose,
    'o|output=s'  => \$output,
    'I|head'      => \$head_only,
    'include'     => \$include_headers,
    'help'        => \$help,
) or die "Usage: whipl [options] <url>\nTry 'whipl --help' for more information.\n";

if ($help) {
    print <<'HELP';
whipl - Fast HTTP/HTTPS client

Usage: whipl [options] <url>

Options:
  -X, --request METHOD   HTTP method (GET, POST, PUT, DELETE, etc.)
  -H, --header HEADER    Custom header (can be used multiple times)
  -d, --data DATA        Request body data
  -u, --user USER:PASS   Basic authentication
  -L, --location         Follow redirects
  -v, --verbose          Verbose output (shows request/response details)
  -o, --output FILE      Save output to file
  -I, --head             Show headers only (uses HEAD method)
      --include          Include headers in output
      --help             Show this help message

Examples:
  whipl http://example.com
  whipl -v https://api.example.com/endpoint
  whipl -X POST -d '{"key":"value"}' https://api.example.com
  whipl -H 'Authorization: Bearer token' https://api.example.com
  whipl -u user:pass https://api.example.com/secure
  whipl -L http://example.com/redirect
  whipl -o output.html http://example.com
  whipl -I http://example.com

HELP
    exit 0;
}

die "Usage: whipl [options] <url>\nTry 'whipl --help' for more information.\n" unless @ARGV;

my $url = $ARGV[0];
$method ||= $head_only ? 'HEAD' : ($data ? 'POST' : 'GET');

my %opts;
$opts{headers} = \@headers if @headers;
$opts{body} = $data if $data && !$head_only;
$opts{follow_redirects} = 1 if $follow;
$opts{verbose} = 1 if $verbose;

if ($user) {
    require MIME::Base64;
    my $auth = MIME::Base64::encode_base64($user, '');
    push @{$opts{headers}}, "Authorization: Basic $auth";
}

my $resp = Whipl::request($method, $url, \%opts);
die "Request failed\n" unless $resp;

if ($resp->{error}) {
    die "Error: $resp->{error}\n";
}

my $out_data = '';
if ($include_headers || $head_only) {
    $out_data .= $resp->{headers} . "\n\n" if $resp->{headers};
}
$out_data .= $resp->{body} if $resp->{body} && !$head_only;

if ($output) {
    open my $fh, '>', $output or die "Cannot write to $output: $!\n";
    binmode $fh;
    print $fh $out_data;
    close $fh;
} else {
    print $out_data;
}
EOF

sed "s|LIBDIR|$LIBDIR|g" /tmp/whipl > /tmp/whipl.final
$SUDO install -m 755 /tmp/whipl.final "$BINDIR/whipl"
rm /tmp/whipl /tmp/whipl.final

echo
echo "Installation complete!"
echo "whipl installed to: $BINDIR/whipl"
echo
echo "Try: whipl --help"
