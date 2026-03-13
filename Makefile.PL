use ExtUtils::MakeMaker;

WriteMakefile(
    NAME           => 'Whipl',
    VERSION_FROM   => 'lib/Whipl.pm',
    XS             => {'xs/whipl.xs' => 'xs/whipl.c'},
    C              => ['xs/whipl.c', 'src/http.c', 'src/transport.c', 'src/buffer.c'],
    OBJECT         => '$(BASEEXT)$(OBJ_EXT) http$(OBJ_EXT) transport$(OBJ_EXT) buffer$(OBJ_EXT)',
    INC            => '-I.',
    OPTIMIZE       => '-O3 -march=native',
);
