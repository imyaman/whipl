use ExtUtils::MakeMaker;

WriteMakefile(
    NAME           => 'Whipl',
    VERSION_FROM   => 'lib/Whipl.pm',
    XS             => {'xs/whipl.xs' => 'Whipl.c'},
    OBJECT         => 'Whipl$(OBJ_EXT) http$(OBJ_EXT) transport$(OBJ_EXT) buffer$(OBJ_EXT)',
    INC            => '-I.',
    LIBS           => ['-lssl -lcrypto'],
    OPTIMIZE       => '-O3 -march=native',
    clean          => { FILES => 'Whipl.c' },
);

sub MY::postamble {
    return <<'MAKE';
Whipl.c: xs/whipl.xs
	$(XSUBPPRUN) $(XSUBPPARGS) xs/whipl.xs > $@

http$(OBJ_EXT): src/http.c src/http.h src/buffer.h src/transport.h
	$(CC) -c $(CCFLAGS) $(OPTIMIZE) $(CCCDLFLAGS) "-I$(PERL_INC)" $(PASTHRU_DEFINE) $(DEFINE) src/http.c -o $@

transport$(OBJ_EXT): src/transport.c src/transport.h
	$(CC) -c $(CCFLAGS) $(OPTIMIZE) $(CCCDLFLAGS) "-I$(PERL_INC)" $(PASTHRU_DEFINE) $(DEFINE) src/transport.c -o $@

buffer$(OBJ_EXT): src/buffer.c src/buffer.h
	$(CC) -c $(CCFLAGS) $(OPTIMIZE) $(CCCDLFLAGS) "-I$(PERL_INC)" $(PASTHRU_DEFINE) $(DEFINE) src/buffer.c -o $@
MAKE
}
