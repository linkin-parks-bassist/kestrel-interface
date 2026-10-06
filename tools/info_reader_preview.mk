bin/lib/info-reader-preview: tools/info_reader_preview.c tools/compile_eff.c components/parser/kest_eff_parser.c $(lib_objdir)/libkest.so $(lib_hdrs)
	gcc $(CFLAGS_LIB) -o $@ $< -L$(lib_objdir) -lkest -lm -Wl,-rpath,'$$ORIGIN'
