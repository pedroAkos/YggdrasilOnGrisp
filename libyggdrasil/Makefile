RTEMS_ROOT ?= $(HOME)/Documents/Grisp/grisp-software/rtems-install/rtems/5
RTEMS_BSP ?= atsamv

include $(RTEMS_ROOT)/make/custom/$(RTEMS_BSP).mk

CPPFLAGS += -Iinclude

LIB = $(BUILDDIR)/libyggdrasil.a
LIB_PIECES = $(wildcard src/*.c)
LIB_PIECES += $(wildcard src/core/*.c)
LIB_PIECES += $(wildcard src/core/protos/*.c)
LIB_PIECES += $(wildcard src/protocols/discovery/*.c)
LIB_PIECES += $(wildcard src/interfaces/*.c)
LIB_PIECES += $(wildcard src/data_structures/generic/*.c)
LIB_PIECES += $(wildcard src/data_structures/specialized/*.c)
LIB_OBJS = $(LIB_PIECES:%.c=$(BUILDDIR)/%.o)
LIB_DEPS = $(LIB_PIECES:%.c=$(BUILDDIR)/%.d)

all: $(BUILDDIR) $(LIB)

install: all
	mkdir -p $(PROJECT_INCLUDE)/yggdrasil
	#mkdir -p $(PROJECT_INCLUDE)/yggdrasil/protos
	#mkdir -p $(PROJECT_INCLUDE)/yggdrasil/interfaces
	#mkdir -p $(PROJECT_INCLUDE)/yggdrasil/data_structures
	#mkdir -p $(PROJECT_INCLUDE)/yggdrasil/data_structures/generic
	#mkdir -p $(PROJECT_INCLUDE)/yggdrasil/data_structures/specialized
	#mkdir -p $(PROJECT_INCLUDE)/yggdrasil/protocols
	#mkdir -p $(PROJECT_INCLUDE)/yggdrasil/protocols/discovery

	install -m 644 $(LIB) $(PROJECT_LIB)
	install -m 644 include/yggdrasil/*.h $(PROJECT_INCLUDE)/yggdrasil
	#install -m 644 include/yggdrasil/protos/*.h $(PROJECT_INCLUDE)/yggdrasil/protos
	#install -m 644 include/yggdrasil/protocols/*.h $(PROJECT_INCLUDE)/yggdrasil/protocols/
	#install -m 644 include/yggdrasil/interfaces/*.h $(PROJECT_INCLUDE)/yggdrasil/interfaces/
	#install -m 644 include/yggdrasil/data_structures/generic/*.h $(PROJECT_INCLUDE)/yggdrasil/data_structures/generic/
	#install -m 644 include/yggdrasil/data_structures/specialized/*.h $(PROJECT_INCLUDE)/yggdrasil/data_structures/specialized/

$(BUILDDIR):
	mkdir $(BUILDDIR)
	mkdir $(BUILDDIR)/src
	mkdir $(BUILDDIR)/src/core
	mkdir $(BUILDDIR)/src/core/protos
	mkdir $(BUILDDIR)/src/protocols
	mkdir $(BUILDDIR)/src/protocols/discovery
	mkdir $(BUILDDIR)/src/data_structures
	mkdir $(BUILDDIR)/src/data_structures/generic
	mkdir $(BUILDDIR)/src/data_structures/specialized
	mkdir $(BUILDDIR)/src/interfaces

$(LIB): $(LIB_OBJS)
	$(AR) rcu $@ $^
	$(RANLIB) $@

clean:
	rm -rf $(BUILDDIR)
	rm -rf $(PROJECT_INCLUDE)/yggdrasil

-include $(LIB_DEPS)
