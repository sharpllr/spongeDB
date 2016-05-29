CC := gcc
libs := -lpthread

sources := $(wildcard include/util/*.c ds/*.c src/*.c *.c)

ut_sources += $(wildcard unit_test/*.c) $(sources)
ut_objects := $(patsubst %.c, %.o, $(ut_sources))

CLI = N			#for client unit test

predef := -DHAVE_ATOMIC -DHAVE_PROC_STAT -DHAVE_PROC_SMAPS 
ae_predef := -DHAVE_EPOLL
test_predef := -D_TERM_PRT_ -D_DBG_VER_
arch_def := -D_POSIX_ -D__ARCH_CPU_X86_FAMILY__
atomic_def := -D_GCC_BUILT_ATOMIC_
#atomic_def := -D_POSIX_MUTEX_ATOMIC_

#for ut-cli/srv switch here
ifeq ("$(CLI)", "Y")
test_predef += -D_UT_CLI_
ut_target := ut-cli
else
ut_target := ut-srv
endif

ifeq ("$(arch)", "32")
CFLAGS := -m32
arch_def += -D_ARCH_X86_32_
else
CFLAGS := -m64 
endif

predef += $(ae_predef) $(test_predef) $(arch_def) $(atomic_def)
header_path := -Iinclude/ -Isrc/

CFLAGS := -g3 -o0 -Wall $(predef) $(header_path) 

all:ut
	$(CC) -o $(ut_target) $(ut_objects) $(libs)

ut:$(ut_objects)
	$(CC) -o $(ut_target) $(ut_objects) $(libs)
	
clean:
	rm -rf $(ut_objects) $(ut_target)
