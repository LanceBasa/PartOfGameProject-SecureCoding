
##########################
## DO NOT SUBMIT THIS FILE
##########################


.PHONY: all clean test docs
.DELETE_ON_ERROR:

# flags used for test code

check_c_flags 	:= $(shell pkg-config --cflags check)
check_ld_flags 	:= $(shell pkg-config --libs check)

######
# variables open to being overridden
#
# override by running e.g.
#
#		CC=clang make all

# enable sanitizers.
# suggest adding ",leak"
safety_features = \
	-fno-omit-frame-pointer \
	-fsanitize=address,undefined \
	-fstack-protector-strong

SHELL 	= bash
CC 			= gcc
override CFLAGS	+= -D_POSIX_C_SOURCE=200809L -g -I. -std=c11 -pedantic -Wall -Wextra -Wconversion \
        $(check_c_flags) $(safety_features)
LDFLAGS = $(check_ld_flags) $(safety_features)

p_and_p.o:

all: p_and_p.o

# Requires that `doxygen` and `dot` are on your PATH.
# Intall with `sudo apt-get doxygen graphviz` if not.
docs: Doxyfile *.c *.h
	doxygen


## test targets

test_modules = \
	check_p_and_p



# one of
# "silent", "minimal", "normal", "verbose"
test_verbosity=verbose

# how to make a .c file from a .ts file
%.c: %.ts
	checkmk $< > $@

# for each test module, the .c file is a target
$(foreach test_module,$(test_modules),$(eval $(test_module).c:))

# for each test module, the .o file is a target
$(foreach test_module,$(test_modules),$(eval $(test_module).o:))

# for each test module, the program is a target
# and the object files test_utils.o, p_and_p.o are a dependency
$(foreach test_module,$(test_modules),$(eval $(test_module): test_utils.o p_and_p.o))

# alter bad builtin rule for linking
# (puts libs in bad order)
%: %.o
	$(CC) -o $@ $^ $(LDFLAGS)

test_c_files = $(test_modules:=.c)

test_o_files = $(test_modules:=.o)

test_progs = $(test_modules)

test_utils.o: test_utils.h

check_p_and_p.o: p_and_p.h

test: $(test_progs)
	@set -e; \
	for test_prog in $(test_progs); do \
		: "set env vars then run test prog" ; \
		$(if $(ck_run_case),CK_RUN_CASE=$(ck_run_case),) \
			CK_FORK=$(ck_fork) \
			CK_VERBOSITY=$(test_verbosity) \
				./$$test_prog; \
	done



clean:
	-rm -rf *.o $(lib) $(test_c_files) $(test_progs) \
            adjust_score.h docs/html


