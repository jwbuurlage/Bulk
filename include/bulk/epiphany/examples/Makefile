# Home directory of the Bulk repository
BULK=../../../..
ESDK=${EPIPHANY_HOME}
ELDF=${ESDK}/bsps/current/fast.ldf

HOST_INCLUDES = -I/usr/include/esdk \
				-I${BULK}/include \
				-I${ESDK}/tools/host/include \
				-I${BULK}/ext/catch/include

E_INCLUDES 	  = -I${BULK}/include


HOST_LIBS = \
	 -L /usr/arm-linux-gnueabihf/lib \
	 -L${ESDK}/tools/host/lib \
	 -L${BULK}/include/bulk/epiphany/lib \
	 -lbulk-epiphany-host \
	 -le-hal \
	 -le-loader

E_LIBS = \
	 -L${ESDK}/tools/host/lib \
	 -L${BULK}/include/bulk/epiphany/lib \
	 -lbulk-epiphany-e \
	 -le-lib

# no-tree-loop-distribute-patters makes sure the compiler
# does NOT replace loops with calls to memcpy, residing in external memory
CFLAGS    = $(HOST_INCLUDES) $(HOST_LIBS) -std=c99 -O3 -Wall -Wfatal-errors
CXXFLAGS  = $(HOST_INCLUDES) $(HOST_LIBS) -std=c++14 -O3 -ffast-math -Wall -Wfatal-errors
E_CFLAGS  = $(E_INCLUDES)    $(E_LIBS) 	  -std=c99 -O3 -fno-strict-aliasing -ffast-math -fno-tree-loop-distribute-patterns -Wall -Wfatal-errors
E_CXXFLAGS= $(E_INCLUDES)    $(E_LIBS) 	  -std=c++14 -Wall -Os -fno-tree-loop-distribute-patterns -fno-exceptions -fno-rtti -fno-threadsafe-statics -freg-struct-return -fno-default-inline

########################################################

# ARCH will be either x86_64, x86, or armv7l (parallella)
ARCH=$(shell uname -m)

ifeq ($(ARCH),x86_64)
ARM_PLATFORM_PREFIX=arm-linux-gnueabihf-
E_PLATFORM_PREFIX  =epiphany-elf-
else
ARM_PLATFORM_PREFIX=
E_PLATFORM_PREFIX  =e-
endif

########################################################

all: basic tests

########################################################

bin/%.elf: %.c
	@echo "CC $<"
	@$(E_PLATFORM_PREFIX)gcc -T ${ELDF} -o $@ $< $(E_CFLAGS)

bin/%.elf: %.cpp
	@echo "CXX $<"
	@$(E_PLATFORM_PREFIX)g++ -T ${ELDF} -o $@ $< $(E_CXXFLAGS)

bin/%.s: %.c
	@echo "CC $<"
	@$(E_PLATFORM_PREFIX)gcc -T $(ELDF) -fverbose-asm -S $< -o $@ $(E_CFLAGS)

bin/%: %.c
	@echo "CC $< blabla"
	@$(ARM_PLATFORM_PREFIX)gcc -o $@ $< $(CFLAGS)

bin/%: %.cpp
	@echo "CXX $<"
	@$(ARM_PLATFORM_PREFIX)g++ -o $@ $< $(CXXFLAGS)

bin/%.o: %.cpp
	@echo "CXX $<"
	@$(ARM_PLATFORM_PREFIX)g++ -c -o $@ $< $(CXXFLAGS)

########################################################

basic: bin/basic bin/basic/host_basic bin/basic/e_basic.elf

bin/basic:
	@mkdir -p bin/basic

#######################################################

tests: bin/tests bin/unittests bin/tests/test_kernel1.elf bin/tests/test_kernel2.elf bin/tests/test_kernel3.elf

bin/tests:
	@mkdir -p bin/tests

bin/unittests: bin/tests/catch_main.o bin/tests/communication.o
	@echo "$@"
	@$(ARM_PLATFORM_PREFIX)g++ -o $@ $^ $(HOST_LIBS)

#######################################################

clean:
	rm -r bin

