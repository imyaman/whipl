package Whipl;

use strict;
use warnings;
use XSLoader;

our $VERSION = '1.0.0';

XSLoader::load('Whipl', $VERSION);

1;

__END__

=head1 NAME

Whipl - Fast HTTP client

=head1 SYNOPSIS

    use Whipl;
    
    my $resp = Whipl::get('http://example.com');
    print $resp->{body};

=cut
