
AM_CFLAGS = -Wall -W -Wpadded -I../$(SRCDIR)/src

INCLUDES = -I$(top_srcdir)/src

bin_PROGRAMS = test-validate_input_move

TESTS = $(bin_PROGRAMS)

test_validate_input_move_SOURCES = test-validate_input_move.c $(top_srcdir)/src/validate_input_move.c $(top_srcdir)/src/validate_input_move.h $(top_srcdir)/src/move_spec.c $(top_srcdir)/src/move_spec.h
test_validate_input_move_LDFLAGS = -L$(top_builddir)/src
test_validate_input_move_LDADD =

