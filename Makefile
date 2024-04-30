# print-clang-ast/Makefile

# --------------------------- Configuration ----------------------------
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

# Let the user override my defaults.
-include pre-config.mk


# -------------------- Helper definitions and scripts ------------------
# CREATE_OUTPUT_DIRECTORY, etc.
include sm-lib.mk

# Python interpreter.
PYTHON3 := python3

# Script to run a program and compare to expected output.
RUN_COMPARE_EXPECT := $(PYTHON3) ./run-compare-expect.py

# The diagram editor program, used for the 'check-diagrams' target (and
# not otherwise, so it's fine if this is missing).  The editor is:
# https://github.com/smcpeak/ded
DED := $(HOME)/wrk/ded/ded

# My script to check some rules.  Not used by default.
CHECK_SRCFILE_RULES := check-srcfile-rules


# --------------------- llvm-config query results ----------------------
# Program to query the various LLVM configuration options.
LLVM_CONFIG := $(CLANG_LLVM_INSTALL_DIR)/bin/llvm-config

# C++ compiler options to ensure ABI compatibility.
LLVM_CXXFLAGS := $(shell $(LLVM_CONFIG) --cxxflags)

# Directory containing the clang library files, both static and dynamic.
LLVM_LIBDIR := $(shell $(LLVM_CONFIG) --libdir)

# Other flags needed for linking, whether statically or dynamically.
LLVM_LDFLAGS_AND_SYSTEM_LIBS := $(shell $(LLVM_CONFIG) --ldflags --system-libs)


# -------------------------- Compiler options --------------------------
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

# Get llvm compilation flags.
#
# Except, remove '-fno-exceptions' because I want to use std::regex but
# that uses exceptions to report invalid regexes.  My understanding is
# this should be OK as long as I don't let exceptions propagate through
# the Clang+LLVM code.
CXXFLAGS += $(filter-out -fno-exceptions,$(LLVM_CXXFLAGS))

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


# ------------------------------ Recipes -------------------------------
# Default target.
all:
.PHONY: all

# Pull in automatic dependencies.
-include $(wildcard *.d)

# Compile a C++ source file.
%.o: %.cc
	$(CXX) -c -o $@ $(GENDEPS_FLAGS) $(CXXFLAGS) $<

# Preprocess a C++ source file, for debugging etc.
%.ii: %.cc
	$(CXX) -E -o $@ $(GENDEPS_FLAGS) $(CXXFLAGS) $<

OBJS :=
OBJS += clang-ast-visitor.o
OBJS += clang-util.o
OBJS += decl-implicit.o
OBJS += enum-util.o
OBJS += file-util-test.o
OBJS += file-util.o
OBJS += number-clang-ast-nodes.o
OBJS += pca-command-line-options-test.o
OBJS += pca-command-line-options.o
OBJS += print-clang-ast-nodes.o
OBJS += print-clang-ast.o
OBJS += printer-visitor.o
OBJS += rav-printer-visitor.o
OBJS += sm-pp-util-test.o
OBJS += stringref-parse-test.o
OBJS += stringref-parse.o
OBJS += trace-test.o
OBJS += trace.o
OBJS += util-test.o
OBJS += util.o

# Executable.
all: print-clang-ast.exe
print-clang-ast.exe: $(OBJS)
	$(CXX) -g -Wall -o $@ $(OBJS) $(LDFLAGS)


# ------------------------------- Tests --------------------------------
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


# -------------------- Tests for --print-ast-nodes ---------------------
# Options for specific source files.  I cannot just pass '-std=c++20'
# for all files due to
# https://github.com/llvm/llvm-project/issues/63959.
FILE_OPTS_bitfield_with_init := -std=c++20
FILE_OPTS_function_requires_requires := -std=c++20

define FILE_OPTS_FOR
$(FILE_OPTS_$(call FILENAME_TO_VARNAME,$(1)))
endef


# General options to PCA for the tests.
PCA_OPTIONS :=
PCA_OPTIONS += --print-ast-nodes
PCA_OPTIONS += --suppress-addresses

# This option is mainly aimed at struct-with-fwd.cc, but should be fine
# for all of them.
PCA_OPTIONS += --force-implicit


# Generate a single JSON output from an input.
out/%.json: in/src/% print-clang-ast.exe
	$(CREATE_OUTPUT_DIRECTORY)
	./print-clang-ast.exe $(PCA_OPTIONS) \
	  $(call FILE_OPTS_FOR,$*) in/src/$* >$@

# Same, but with abbreviated field names.
out/%.abbrev.json: in/src/% print-clang-ast.exe
	$(CREATE_OUTPUT_DIRECTORY)
	./print-clang-ast.exe $(PCA_OPTIONS) --no-ast-field-qualifiers \
	  $(call FILE_OPTS_FOR,$*) in/src/$* >$@


# Inputs.
TEST_INPUTS := $(wildcard in/src/*.cc)

# A few tests do not work in Clang-16 due to
# https://github.com/llvm/llvm-project/issues/60778
ifneq ($(USE_SOURCE_BUILD),1)
TEST_INPUTS := $(filter-out in/src/ct-cont-ct-pmspec.cc,$(TEST_INPUTS))
TEST_INPUTS := $(filter-out in/src/ct-cont-ct-emspec-of-cspspec.cc,$(TEST_INPUTS))
endif

# Outputs.
TEST_OUTPUTS := $(patsubst in/src/%,out/%.json,$(TEST_INPUTS))
TEST_ABBREV_OUTPUTS := $(patsubst in/src/%,out/%.abbrev.json,$(TEST_INPUTS))

.PHONY: test-outputs
test-outputs: $(TEST_OUTPUTS)
check: test-outputs

.PHONY: test-abbrev-outputs
test-abbrev-outputs: $(TEST_ABBREV_OUTPUTS)
check: test-abbrev-outputs


# Check that the output matches expected output.
#
# The --drop-lines allow the same output to work across Clang versions
# (currently I'm only testing with Clang 16 and 17):
#
# * ODRHash depends on the Clang version.
#
# * 'IsDefaulted' and 'IndexInContext' are only available in Clang 17+.
#
# * 'InjectedArgs' and 'DNLoc' changed in Clang 17.
#
# * I've been making changes to the 'EndRangeLoc' computation in my
#   personal copy of Clang 17.
#
out/%.nodes: out/%.json in/exp/%.nodes print-clang-ast.exe
	$(CREATE_OUTPUT_DIRECTORY)
	$(RUN_COMPARE_EXPECT) \
	  --actual $@ --expect in/exp/$*.nodes \
	  --drop-lines 'ODRHash":' \
	  --drop-lines 'IsDefaulted":' \
	  --drop-lines 'IndexInContext":' \
	  --drop-lines 'InjectedArgs(...)?":' \
	  --drop-lines 'EndRangeLoc":' \
	  --drop-lines 'fixedEndLoc.*":' \
	  --drop-lines 'DNLoc":' \
	  cat out/$*.json


# Check outputs against expected output.
TEST_CONFIRMATIONS := $(patsubst in/src/%,out/%.nodes,$(TEST_INPUTS))

.PHONY: check-nodes
check-nodes: $(TEST_CONFIRMATIONS)

check: check-nodes


# ---------------------- Check test syntax rules -----------------------
# Check that the test source files follow my rules.
#
# Passing CHECK_SRCFILE_RULES_ARGS=--fix enables automatic fixes.
out/%.cc.rules: in/src/%.cc
	$(CREATE_OUTPUT_DIRECTORY)
	cd in/src && $(CHECK_SRCFILE_RULES) $(CHECK_SRCFILE_RULES_ARGS) $*.cc
	touch $@

# This is not run by default since it requires my script
.PHONY: check-srcfile-rules
check-srcfile-rules: $(patsubst in/src/%,out/%.rules,$(TEST_INPUTS))


# ----------------------- Test ded --check-graph -----------------------
# Check that a diagram's graph agrees with the diagram and with the
# graph source.
#
# This depends on the JSON file because --check-graph-source reads it.
# Here, I am just assuming that the diagrams are following the proper
# naming convention regarding where their JSON source is.
out/%.ded.cg: doc/ASTsForTemplatesImages/%.ded out/%.cc.abbrev.json
	$(CREATE_OUTPUT_DIRECTORY)
	$(DED) --check-graph --check-graph-source \
	  doc/ASTsForTemplatesImages/$*.ded
	touch $@

# The diagrams that have "-types" in their names use the same JSON
# source as the ones without "-types", but the Makefile rule does not
# know that.  So, this rule lets 'make' think it can supply the needed
# file even though it actually will not be used.
out/%-types.cc.abbrev.json: out/%.cc.abbrev.json
	cp $^ $@


# The 'check-diagrams' target is *not* part of 'check' because that
# would require having 'ded', which I do not want to require.
#
# 2023-09-11: This is broken for the moment because the graphs are now
# slightly different even though the diagram images should be
# unaffected.  I need to modify 'ded' so it can easily add new graph
# data and check image equality.
.PHONY: check-diagrams

# The diagrams are listed in the order they appear in ASTsForTemplates.rst.
CHECKED_DIAGRAMS :=
CHECKED_DIAGRAMS += ft-defn.ded
CHECKED_DIAGRAMS += ft-inst.ded
CHECKED_DIAGRAMS += oc-cont-ft-defn.ded
CHECKED_DIAGRAMS += ct-defn.ded
CHECKED_DIAGRAMS += ct-defn-types.ded
CHECKED_DIAGRAMS += ct-inst.ded
CHECKED_DIAGRAMS += ct-inst-types.ded
CHECKED_DIAGRAMS += ct-cont-of-defn.ded
CHECKED_DIAGRAMS += ct-cont-of-inst.ded
CHECKED_DIAGRAMS += ft-espec.ded
CHECKED_DIAGRAMS += ct-espec.ded
CHECKED_DIAGRAMS += ct-pspec.ded
CHECKED_DIAGRAMS += ct-cont-of-espec.ded
CHECKED_DIAGRAMS += ct-cont-ft-defn.ded
CHECKED_DIAGRAMS += ct-cont-ft-inst.ded
CHECKED_DIAGRAMS += ct-cont-ft-espec.ded
CHECKED_DIAGRAMS += ct-cont-ft-emspec.ded
CHECKED_DIAGRAMS += ct-cont-ft-csspec.ded
CHECKED_DIAGRAMS += ct-cont-ct-inst.ded
CHECKED_DIAGRAMS += ct-cont-ct-espec.ded
CHECKED_DIAGRAMS += ct-cont-ct-pspec.ded
CHECKED_DIAGRAMS += ct-cont-ct-emspec.ded
CHECKED_DIAGRAMS += ct-cont-ct-pmspec.ded
CHECKED_DIAGRAMS += ct-cont-ct-csspec.ded
CHECKED_DIAGRAMS += ct-cont-ct-cspspec.ded

check-diagrams: $(patsubst %,out/%.cg,$(CHECKED_DIAGRAMS))


# ----------------------- Test --printer-visitor -----------------------
in/exp/pv/%.pv:
	touch $@

out/pv/%.pv: in/src/% in/exp/pv/%.pv print-clang-ast.exe
	$(CREATE_OUTPUT_DIRECTORY)
	$(RUN_COMPARE_EXPECT) \
	  --actual $@ \
	  --expect in/exp/pv/$*.pv \
	  ./print-clang-ast.exe --printer-visitor \
	    --print-visit-context \
	    --print-implicit-qual-types \
	    -xc++ in/src/$*

PRINTER_VISITOR_TESTS :=
PRINTER_VISITOR_TESTS += ct-inst.cc
PRINTER_VISITOR_TESTS += expr-array-size.cc
PRINTER_VISITOR_TESTS += friend-decl.cc
PRINTER_VISITOR_TESTS += friend-template-decl.cc

.PHONY: check-printer-visitor
check-printer-visitor: $(patsubst %,out/pv/%.pv,$(PRINTER_VISITOR_TESTS))

check: check-printer-visitor


# --------------------- Test --rav-printer-visitor ---------------------
out/rpv/%.rpv.ok: in/src/% print-clang-ast.exe
	$(CREATE_OUTPUT_DIRECTORY)
	@#
	@# Run --rav-printer-visitor.
	./print-clang-ast.exe --rav-printer-visitor -xc++ \
	  $(call FILE_OPTS_FOR,$*) in/src/$* > out/rpv/$*.rpv
	@#
	@# Run --printer-visitor with the RAV compatibility flag.
	./print-clang-ast.exe --printer-visitor --omit-ctpsd-taw -xc++ \
	  $(call FILE_OPTS_FOR,$*) in/src/$* > out/rpv/$*.pv
	@#
	@# Check that they agree.
	diff -u out/rpv/$*.rpv out/rpv/$*.pv
	@#
	@# Indicate success.
	touch $@

RAV_PRINTER_VISITOR_TESTS :=
RAV_PRINTER_VISITOR_TESTS += bitfield-with-init.cc
RAV_PRINTER_VISITOR_TESTS += call-fn-via-using-decl.cc
RAV_PRINTER_VISITOR_TESTS += call-op-via-using-decl.cc
RAV_PRINTER_VISITOR_TESTS += ct-cont-ct-cspspec.cc
RAV_PRINTER_VISITOR_TESTS += ct-cont-ct-csspec.cc
RAV_PRINTER_VISITOR_TESTS += ct-cont-ct-defn-ool.cc
RAV_PRINTER_VISITOR_TESTS += ct-cont-ct-emspec.cc
RAV_PRINTER_VISITOR_TESTS += ct-cont-ct-emspec-of-cspspec.cc
RAV_PRINTER_VISITOR_TESTS += ct-cont-ct-espec.cc
RAV_PRINTER_VISITOR_TESTS += ct-cont-ct-inst.cc
RAV_PRINTER_VISITOR_TESTS += ct-cont-ct-pmspec.cc
RAV_PRINTER_VISITOR_TESTS += ct-cont-ct-pspec.cc
RAV_PRINTER_VISITOR_TESTS += ct-cont-friend-ft-spec-inst.cc
RAV_PRINTER_VISITOR_TESTS += ct-cont-ft-csspec.cc
RAV_PRINTER_VISITOR_TESTS += ct-cont-ft-defn.cc
RAV_PRINTER_VISITOR_TESTS += ct-cont-ft-emspec.cc
RAV_PRINTER_VISITOR_TESTS += ct-cont-ft-espec.cc
RAV_PRINTER_VISITOR_TESTS += ct-cont-ft-inst.cc

# Batch of tests I am working on.
ifeq (0,1)
RAV_PRINTER_VISITOR_TESTS += ct-cont-oc-emspec.cc
RAV_PRINTER_VISITOR_TESTS += ct-cont-oc-inst.cc
RAV_PRINTER_VISITOR_TESTS += ct-cont-of-defn.cc
RAV_PRINTER_VISITOR_TESTS += ct-cont-of-espec.cc
RAV_PRINTER_VISITOR_TESTS += ct-cont-of-inst.cc
RAV_PRINTER_VISITOR_TESTS += ct-defn.cc
RAV_PRINTER_VISITOR_TESTS += ct-espec.cc
RAV_PRINTER_VISITOR_TESTS += ct-inner-struct.cc
endif

RAV_PRINTER_VISITOR_TESTS += ct-inst.cc
RAV_PRINTER_VISITOR_TESTS += expr-array-size.cc
RAV_PRINTER_VISITOR_TESTS += friend-decl.cc
RAV_PRINTER_VISITOR_TESTS += friend-template-decl.cc
RAV_PRINTER_VISITOR_TESTS += functiondecl-body.cc
RAV_PRINTER_VISITOR_TESTS += ool-defn-tmethod-tclass.cc


.PHONY: check-rav-printer-visitor
check-rav-printer-visitor: $(patsubst %,out/rpv/%.rpv.ok,$(RAV_PRINTER_VISITOR_TESTS))

check: check-rav-printer-visitor


# ------------------------ 'check-full' target -------------------------
# Check all the optional stuff too.
.PHONY: check-full
check-full: check
check-full: check-srcfile-rules
check-full: check-diagrams


# --------------------------- 'clean' target ---------------------------
.PHONY: clean
clean:
	$(RM) *.o *.d *.exe
	$(RM) -r out


# EOF
