# print-clang-ast/Makefile

# Default target.
all:
.PHONY: all


# ---- Configuration ----
# Set to 1 if I am using a build from source, 0 for a binary
# distribution.
USE_SOURCE_BUILD := 0

ifeq ($(USE_SOURCE_BUILD),1)
  # Trying my own build.
  CLANG_LLVM_SRC_DIR = $(HOME)/bld/llvm-project-2023-07-14
  CLANG_LLVM_INSTALL_DIR = $(CLANG_LLVM_SRC_DIR)/build

else
  # Installation directory from a binary distribution.
  # Has five subdirectories: bin include lib libexec share.
  # Downloaded from: https://github.com/llvm/llvm-project/releases/download/llvmorg-14.0.0/clang+llvm-14.0.0-x86_64-linux-gnu-ubuntu-18.04.tar.xz
  #CLANG_LLVM_INSTALL_DIR = $(HOME)/opt/clang+llvm-14.0.0-x86_64-linux-gnu-ubuntu-18.04
  #
  # In Clang+LLVM 14, CFGImplicitDtor::getDestructorDecl is not fully
  # implemented.  So I'm switching to 16.
  CLANG_LLVM_INSTALL_DIR = $(HOME)/opt/clang+llvm-16.0.0-x86_64-linux-gnu-ubuntu-18.04

endif

# smbase directory to get a few helper files.
SMBASE_DIR := $(HOME)/wrk/smbase


# ---- Helper definitions and scripts ----
# Get definition of CREATE_OUTPUT_DIRECTORY, etc.
include $(SMBASE_DIR)/sm-lib.mk

PYTHON3 := python3
RUN_COMPARE_EXPECT := $(PYTHON3) $(SMBASE_DIR)/run-compare-expect.py


# ---- llvm-config query results ----
# Program to query the various LLVM configuration options.
LLVM_CONFIG := $(CLANG_LLVM_INSTALL_DIR)/bin/llvm-config

# C++ compiler options to ensure ABI compatibility.
LLVM_CXXFLAGS := $(shell $(LLVM_CONFIG) --cxxflags)

# Directory containing the clang library files, both static and dynamic.
LLVM_LIBDIR := $(shell $(LLVM_CONFIG) --libdir)

# Other flags needed for linking, whether statically or dynamically.
LLVM_LDFLAGS_AND_SYSTEM_LIBS := $(shell $(LLVM_CONFIG) --ldflags --system-libs)


# ---- Compiler options ----
# C++ compiler.
#CXX = g++
CXX = $(CLANG_LLVM_INSTALL_DIR)/bin/clang++

# Compiler options, including preprocessor options.
CXXFLAGS =

# Without optimization, adding -g increases compile time by ~20%.
#
# This switch really only works when using g++, since gdb cannot read
# the debug information produced by clang++.  But, the 'lldb' program
# (part of clang+llvm) can read them.
#CXXFLAGS += -g

# Without -g, this increases compile time by ~10%.  With -g -O2, the
# increase is ~50% over not having either.
#CXXFLAGS += -O2

CXXFLAGS += -Wall
CXXFLAGS += -Werror

# Silence a warning about a multi-line comment in DeclOpenMP.h.
CXXFLAGS += -Wno-comment

# Pull in smbase so I can use sm-pp-util.h.
CXXFLAGS += -I$(SMBASE_DIR)

# Get llvm compilation flags.
CXXFLAGS += $(LLVM_CXXFLAGS)

ifeq ($(USE_SOURCE_BUILD),1)
  # When using my own build, I need to separately point at clang includes.
  CXXFLAGS += -I$(CLANG_LLVM_SRC_DIR)/clang/include
  CXXFLAGS += -I$(CLANG_LLVM_INSTALL_DIR)/tools/clang/include
endif

# Tell the source code where the clang installation directory is.
CXXFLAGS += -DCLANG_LLVM_INSTALL_DIR='"$(CLANG_LLVM_INSTALL_DIR)"'

# Switch to enable creation of .d files.
GENDEPS_FLAGS = -MMD


# Linker options.
LDFLAGS =

# Pull in clang+llvm via libclang-cpp.so, which has everything, but is
# only available as a dynamic library.
LDFLAGS += -lclang-cpp

# Arrange for the compiled binary to search the libdir for that library.
# Otherwise, one can set the LD_LIBRARY_PATH envvar before running it.
# Note: the -rpath switch does not work on Windows.
LDFLAGS += -Wl,-rpath=$(LLVM_LIBDIR)

# It appears that llvm::raw_os_ostream::~raw_os_ostream is missing from
# libclang-cpp, so I have to link with LLVMSupport statically.
LDFLAGS += -lLLVMSupport

# Get the needed -L search path, plus things like -ldl.
LDFLAGS += $(LLVM_LDFLAGS_AND_SYSTEM_LIBS)

# Optional custom modifications.
-include config.mk


# ---- Recipes ----
# Pull in automatic dependencies.
-include $(wildcard *.d)

# Compile a C++ source file.
%.o: %.cc
	$(CXX) -c -o $@ $(GENDEPS_FLAGS) $(CXXFLAGS) $<

OBJS :=
OBJS += clang-util.o
OBJS += enum-util.o
OBJS += file-util.o
OBJS += file-util-test.o
OBJS += number-clang-ast-nodes.o
OBJS += pca-command-line-options.o
OBJS += pca-command-line-options-test.o
OBJS += print-clang-ast.o
OBJS += print-clang-ast-nodes.o
OBJS += stringref-parse.o
OBJS += stringref-parse-test.o
OBJS += util.o
OBJS += util-test.o

# Executable.
all: print-clang-ast.exe
print-clang-ast.exe: $(OBJS)
	$(CXX) -g -Wall -o $@ $(OBJS) $(LDFLAGS)


# ---- Tests ----
# Create an empty expected output file if needed.
in/exp/%:
	touch $@


# All tests.
.PHONY: check


# Unit tests.
.PHONY: check-unit
check: out/unit-tests.ok
out/unit-tests.ok: print-clang-ast.exe
	$(CREATE_OUTPUT_DIRECTORY)
	./print-clang-ast.exe --unit-tests
	touch $@

# Test --print-ast-nodes.  This passes -std=c++20 because one test needs
# that and it shouldn't cause problems for the others.
out/%.nodes: in/src/% in/exp/%.nodes print-clang-ast.exe
	$(CREATE_OUTPUT_DIRECTORY)
	$(RUN_COMPARE_EXPECT) \
	  --actual $@ --expect in/exp/$*.nodes \
	  ./print-clang-ast.exe --print-ast-nodes --suppress-addresses \
	    -std=c++20 in/src/$*

.PHONY: check-nodes

check-nodes: out/bitfield-with-init.cc.nodes
check-nodes: out/class-template-class-template-instantiation.cc.nodes
check-nodes: out/class-template-class-template-partial-specialization.cc.nodes
check-nodes: out/class-template-class-template-specialization.cc.nodes
check-nodes: out/class-template-explicit-specialization.cc.nodes
check-nodes: out/class-template-inner-struct.cc.nodes
check-nodes: out/class-template-instantiation.cc.nodes
check-nodes: out/class-template-member-explicit-specialization.cc.nodes
check-nodes: out/class-template-method-template-specialization.cc.nodes
check-nodes: out/class-template-method-template.cc.nodes
check-nodes: out/class-template-method.cc.nodes
check-nodes: out/class-template-only.cc.nodes
check-nodes: out/class-template-ovl-method-canttpt.cc.nodes
check-nodes: out/class-template-partial-specialization.cc.nodes
check-nodes: out/class-template-redecl.cc.nodes
check-nodes: out/declrefexpr-template-args.cc.nodes
check-nodes: out/declrefexpr.cc.nodes
check-nodes: out/default-args.cc.nodes
check-nodes: out/deleted-function.cc.nodes
check-nodes: out/funcptr-param.cc.nodes
check-nodes: out/function-requires-requires.cc.nodes
check-nodes: out/function-template-explicit-specialization.cc.nodes
check-nodes: out/function-template.cc.nodes
check-nodes: out/functiondecl-body.cc.nodes
check-nodes: out/has-defaulted-func-info.cc.nodes
check-nodes: out/method-template-of-concrete-class.cc.nodes
check-nodes: out/nested-function.cc.nodes
check-nodes: out/no-qualifiers.cc.nodes
check-nodes: out/ool-defn-tmethod-tclass.cc.nodes
check-nodes: out/triv-template-function.cc.nodes

check: check-nodes


.PHONY: clean
clean:
	$(RM) *.o *.d *.exe
	$(RM) -r out


# EOF
