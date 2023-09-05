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


# Compilation flags to pass specifically when checking that the output
# of header analysis compiles.
CHECK_OUTPUT_CXXFLAGS :=

# These warnings will fire spuriously when a header file is passed as
# the "main file" of a translation unit, so turn them off.
#
# Note that print-clang-ast.exe internally disables these flags itself,
# so we do not need to pass it to HA, only to $(CXX).
CHECK_OUTPUT_CXXFLAGS += -Wno-unused-function
CHECK_OUTPUT_CXXFLAGS += -Wno-unused-const-variable


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

# Given a directory in $(1) that we will chdir into, return a path that
# can be used to chdir back to the starting point.  (This is currently
# not general; it simply detects specific directories.)
REVDIR = $(subst in/src,../..,$(1))

# Given a name in $(1) that might contain a '%' pattern placeholder,
# replace it with '$*' so it expands to the string matched by the '%'.
UNPCT = $(subst %,$$*,$(1))

# Create a rule that runs the analysis as part of the tests and compare
# to expected output.
#
# Params:
#   $(1): Source file name without directory info, but possibly with a
#         '%' pattern placeholder
#   $(2): Directory containing source file.
#   $(3): Flags for print-clang-ast.exe.
#   $(4): Compilation flags.
define MAKE_RUN_RULE

out/$(1).ha: print-clang-ast.exe $(2)/$(1) in/exp/$(1).ha
	$$(CREATE_OUTPUT_DIRECTORY)
	@#
	@# Run the analysis.
	cd $(2) && $$(call REVDIR,$(2))/print-clang-ast.exe \
	  --outdir=$$(call REVDIR,$(2))/out $(3) $(4) $(call UNPCT,$(1))
	@#
	@# Check that the output (if created) can be compiled.  We
	@# check both Data (alone) and Ops.
	$(if $(findstring .h,$(1)), \
	  cd out && $(CXX) -c -o /dev/null $(4) -I../$(2) \
	    $$(CHECK_OUTPUT_CXXFLAGS) \
	    $(patsubst %.h,%,$(call UNPCT,$(1)))_Data.h \
	)
	$(if $(findstring .h,$(1)), \
	  cd out && $(CXX) -c -o /dev/null $(4) -I../$(2) \
	    $$(CHECK_OUTPUT_CXXFLAGS) \
	    $(patsubst %.h,%,$(call UNPCT,$(1)))_Ops.h \
	)
	@#
	@# Compare output to expectation.
	$$(RUN_COMPARE_EXPECT) \
	  --actual $$@ --expect in/exp/$(call UNPCT,$(1)).ha \
	  cat out/$(call UNPCT,$(1)).deps \
	      out/$(call UNPCT,$(1)).counts \
	      $(if $(findstring .h,$(1)), \
	        out/$(patsubst %.h,%,$(call UNPCT,$(1)))_Names.h \
	        out/$(patsubst %.h,%,$(call UNPCT,$(1)))_Data.h \
	        out/$(patsubst %.h,%,$(call UNPCT,$(1)))_Ops.h \
	        $(if $(findstring --rewrite,$(3)), \
	          out/$(call UNPCT,$(1)) \
	        ) \
	      )

endef

$(eval $(call MAKE_RUN_RULE,%.c,in/src,,))
$(eval $(call MAKE_RUN_RULE,%.cc,in/src,,$(CXXFLAGS)))
$(eval $(call MAKE_RUN_RULE,%.h,in/src,--rewrite,$(CXXFLAGS) -xc++))

$(eval $(call MAKE_RUN_RULE,%.cc,.,,$(CXXFLAGS)))
$(eval $(call MAKE_RUN_RULE,%.h,.,--rewrite,$(CXXFLAGS) -xc++))


# Normal tests.
.PHONY: check

# Tests that currently are broken.
.PHONY: check-broken


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
