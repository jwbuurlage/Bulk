# Home directory of the Bulk repository
BULK=../../..
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
	 -L${BULK}/backends/epiphany/lib \
	 -lbulk-epiphany-host \
	 -le-hal \
	 -le-loader

E_LIBS = \
	 -L${ESDK}/tools/host/lib \
	 -L${BULK}/backends/epiphany/lib \
	 -lbulk-epiphany-e \
	 -le-lib

# no-tree-loop-distribute-patters makes sure the compiler
# does NOT replace loops with calls to memcpy, residing in external memory
CFLAGS    = $(HOST_INCLUDES) $(HOST_LIBS) -std=c99 -O3 -Wall -Wfatal-errors
CXXFLAGS  = $(HOST_INCLUDES) $(HOST_LIBS) -std=c++14 -O3 -ffast-math -Wall -Wfatal-errors
E_CFLAGS  = $(E_INCLUDES)    $(E_LIBS) 	  -std=c99 -O3 -fno-strict-aliasing -ffast-math -fno-tree-loop-distribute-patterns -Wall -Wfatal-errors
E_CXXFLAGS= $(E_INCLUDES)    $(E_LIBS) 	  -std=c++14 -Wall -Os -fno-tree-loop-distribute-patterns -fno-exceptions -fno-rtti -fno-threadsafe-statics -freg-struct-return -fno-default-inline
TOOL_FLAGS= $(HOST_INCLUDES) $(E_INCLUDES) -std=c++14 -O3 -Wall

HOST_DEPS = $(BULK)/backends/epiphany/lib/libbulk-epiphany-host.a \
			$(BULK)/backends/epiphany/include/environment_provider.hpp

E_DEPS    = $(BULK)/backends/epiphany/lib/libbulk-epiphany-e.a \
			$(wildcard $(BULK)/backends/epiphany/include/*.hpp)

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

all: basic benchmark cannon tests

########################################################

bin/%.elf: %.c $(E_DEPS)
	@echo "CC $<"
	@$(E_PLATFORM_PREFIX)gcc -T $(ELDF) -o $@ $< $(E_CFLAGS)

bin/%.elf: %.cpp $(E_DEPS)
	@echo "CXX $<"
	@$(E_PLATFORM_PREFIX)g++ -T $(ELDF) -o $@ $< $(E_CXXFLAGS)

bin/%.s: %.c $(E_DEPS)
	@echo "CC $<"
	@$(E_PLATFORM_PREFIX)gcc -T $(ELDF) -fverbose-asm -S $< -o $@ $(E_CFLAGS)

bin/%: %.c $(HOST_DEPS)
	@echo "CC $<"
	@$(ARM_PLATFORM_PREFIX)gcc -o $@ $< $(CFLAGS)

bin/%: %.cpp $(HOST_DEPS)
	@echo "CXX $<"
	@$(ARM_PLATFORM_PREFIX)g++ -o $@ $< $(CXXFLAGS)

#
# This section uses bulk-compile-tool to compile source files
# that have kernels written as lambdas
#
#
# Say we want to create main.o from the source main.cpp
#
# bulk-compile-tool will generate the following files
# main.cpp.host.cpp     <-- the host file with lambda's replaced
# main.cpp.0.kernel.cpp <-- file containing the first lambda
# main.cpp.1.kernel.cpp <-- file containing the second lambda
#
# We have to compile these into
# main.cpp.host.o
# main.cpp.0.kernel.elf.o
# main.cpp.1.kernel.elf.o
#
# and then link all of those into main.o
#
# To build all of these, we have the following structure
# main.o             depends on  main.cpp.host.o
# main.cpp.host.o    depends on  main.cpp.host.cpp
# main.cpp.host.cpp  depends on  main.cpp
#
# Building main.cpp.host.cpp will also build all kernels
# Building main.o will link in any additional kernels
#
# bulk-compile-tool writes a list of kernel cpp files to stdout
# We first compile
# main.cpp.0.kernel.cpp  to  main.cpp.0.kernel.elf
# Then objcopy
# main.cpp.0.kernel.elf  to  main.cpp.0.kernel.elf.o
# These last ones are then linked into main.o
#

bin/%.o: bin/%.cpp.host.o
	@echo "Linking host object with any kernels"
	@$(ARM_PLATFORM_PREFIX)ld -r -o $@ $< $(KERNEL_LIST_OBJ)
	@echo $(KERNEL_LIST_OBJ) | xargs --no-run-if-empty rm

bin/%.cpp.host.o: %.cpp.host.cpp
	@echo "CXX $<"
	@$(ARM_PLATFORM_PREFIX)g++ -c -o $@ $< $(CXXFLAGS)

%.cpp.host.cpp: %.cpp $(HOST_DEPS)
	@echo "bulk-compile-tool $<"
	$(eval KERNEL_LIST_CPP := $(shell bulk-compile-tool $< -- $(TOOL_FLAGS)))
	$(eval KERNEL_LIST_OBJ := $(KERNEL_LIST_CPP:%.cpp=%.elf.o))
	@echo $(KERNEL_LIST_OBJ) | xargs --no-run-if-empty make
	@echo $(KERNEL_LIST_CPP) | xargs --no-run-if-empty rm

#%.cpp.host.cpp: %.cpp $(HOST_DEPS)
#	@echo "bulk-compile-tool $<"
#	bulk-compile-tool $< -- $(TOOL_FLAGS) | sed s/.cpp$$/.elf.o/g | xargs --no-run-if-empty make

%.kernel.elf.o: %.kernel.elf $(E_DEPS)
	@$(ARM_PLATFORM_PREFIX)objcopy -I binary -O elf32-littlearm -B arm $< $@

%.kernel.elf: %.kernel.cpp $(E_DEPS)
	@echo "Compiling kernel $<"
	@$(E_PLATFORM_PREFIX)g++ -T $(ELDF) -o $@ $< $(E_CXXFLAGS)

########################################################

basic: bin/basic bin/basic/host_basic bin/basic/e_basic.elf

bin/basic:
	@mkdir -p bin/basic

########################################################

cannon: bin/cannon bin/cannon/host_cannon bin/cannon/e_cannon.elf

bin/cannon:
	@mkdir -p bin/cannon


########################################################

benchmark: bin/benchmark bin/benchmark/host_benchmark bin/benchmark/e_benchmark.elf

bin/benchmark:
	@mkdir -p bin/benchmark

#######################################################

tests: bin/tests bin/unittests bin/tests/test_kernel1.elf bin/tests/test_kernel2.elf bin/tests/test_kernel3.elf

bin/tests:
	@mkdir -p bin/tests

bin/unittests: bin/tests/catch_main.o bin/tests/communication.o bin/tests/lambda_kernels.o bin/tests/streaming.o bin/tests/dma.o
	@echo "CXX $@"
	@$(ARM_PLATFORM_PREFIX)g++ -o $@ $^ $(HOST_LIBS)

#######################################################

clean:
	rm -r bin

