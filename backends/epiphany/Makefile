ESDK=${EPIPHANY_HOME}

HOST_LIBNAME = libbulk-epiphany-host.a
E_LIBNAME	 = libbulk-epiphany-e.a

HOST_HEADERS = \
		include/environment_provider.hpp

HOST_SRCS = \
		environment_provider.cpp

E_HEADERS = \
		include/backend.hpp \
		include/coarray.hpp \
		include/combuf.hpp \
		include/dma.hpp \
		include/epiphany_internals.hpp \
		include/messages.hpp \
		include/stream.hpp \
		include/utility.hpp \
		include/variable.hpp \
		include/world_provider.hpp \
		include/world_state.hpp

E_SRCS = \
		backend.cpp \
		dma.cpp \
		get_raw_time.s \
		malloc.cpp \
		reduce_binary_size.cpp \
		stream.cpp \
		utility.cpp \
		world_state.cpp

HOST_INCLUDES = -I./include -I../../include -I${ESDK}/tools/host/include
E_INCLUDES 	  = -I./include -I../../include

HOST_LIBS = -L${ESDK}/tools/host/lib -le-hal
E_LIBS    = -le-lib

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

all: host e

debug: CFLAGS += -DDEBUG -g
debug: CXXFLAGS += -DDEBUG -g
debug: E_CFLAGS += -DDEBUG
debug: E_CXXFLAGS += -DDEBUG
debug: host e

########################################################

E_OBJS    = $(E_SRCS:%.c=build/e/%.o) $(E_SRCS:%.cpp=build/e/%.o) $(E_SRCS:%.s=build/e/%.o)
HOST_OBJS = $(HOST_SRCS:%.c=build/host/%.o) $(HOST_SRCS:%.cpp=build/host/%.o)

vpath %.c src
vpath %.cpp src
vpath %.s src

build/host/%.o: %.c $(HOST_HEADERS)
	@echo "CC $<"
	@$(ARM_PLATFORM_PREFIX)gcc $(CFLAGS) -c $< -o $@

build/host/%.o: %.cpp $(HOST_HEADERS)
	@echo "CXX $<"
	@$(ARM_PLATFORM_PREFIX)g++ $(CXXFLAGS) -c $< -o $@

build/e/%.o: %.c $(E_HEADERS)
	@echo "CC $<"
	@$(E_PLATFORM_PREFIX)gcc $(E_CFLAGS) -c $< -o $@

build/e/%.o: %.cpp $(E_HEADERS)
	@echo "CXX $<"
	@$(E_PLATFORM_PREFIX)g++ $(E_CXXFLAGS) -c $< -o $@

build/e/%.o: %.s $(E_HEADERS)
	@echo "CC $<"
	@$(E_PLATFORM_PREFIX)gcc $(E_CFLAGS) -c $< -o $@

lib/$(HOST_LIBNAME): $(HOST_OBJS)
	@$(ARM_PLATFORM_PREFIX)ar rs $@ $^ 

lib/$(E_LIBNAME): $(E_OBJS)
	@$(E_PLATFORM_PREFIX)ar rs $@ $^ 


debug: CFLAGS += -DDEBUG -g
debug: E_CFLAGS += -DDEBUG
debug: host e

host: host_dirs lib/$(HOST_LIBNAME)

e: e_dirs lib/$(E_LIBNAME)

host_dirs:
	@mkdir -p build/host lib

e_dirs:
	@mkdir -p build/e lib

########################################################

clean:
	rm -r lib
	rm -r build
