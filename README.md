# Print Clang AST

This repo contains the `print-clang-ast.exe` tool, which prints details
about the Clang AST obtained by parsing a single translation unit.

The output is JSON that is intended to be either reviewed manually or
imported into [`ded`](https://github.com/smcpeak/ded).

The eventual goal is to print all AST nodes, but currently it only
prints things relevant to C++ templates.

The output describes two main things:

1. The private data stored in each AST node.

2. The result of calling the public API functions.

The former is the most important, since it allows understanding the
design of the AST.  The latter is mainly done in an ad-hoc manner to
help learn the API capabilities and compare that to the data actually
stored.

In order to print the private data, this tool resorts to a variety of
"dirty tricks" to bypass C++ access control.  That makes it fairly
sensitive to Clang version changes.

# Compiling

Create a file called `pre-config.mk` with contents that point at a
Clang+LLVM installation directory, for example:

```
CLANG_LLVM_INSTALL_DIR = $(HOME)/opt/clang+llvm-16.0.0-x86_64-linux-gnu-ubuntu-18.04
```

Alternatively, to use a source build directory, point it at that
directory like this:

```
USE_SOURCE_BUILD = 1
CLANG_LLVM_SRC_DIR = $(HOME)/bld/llvm-project-2023-07-14
CLANG_LLVM_INSTALL_DIR = $(CLANG_LLVM_SRC_DIR)/build
```

Then:

```
$ make
```

should do it.

After compiling, run:

```
$ make check
```

to run the tests.

# Usage

The basic usage is:

```
$ ./print-clang-ast.exe --print-ast-nodes test.cc >test.json
```

The output is voluminous, so this is mainly intended to be used on tiny
source files that do not `#include` anything.

By default, the program uses fairly verbose names for attributes, which
is helpful for learning the organization of the AST, but creates a lot
of clutter in a diagram.  Consider adding `--no-ast-field-qualifiers` to
the command line to reduce the verbosity.

# Importing into ded

After generating JSON, to import into
[`ded`](https://github.com/smcpeak/ded):

* Start `ded`.

* Choose "Diagram -> Edit object graph..." from the menu.

  * Copy+paste the JSON text into the window that opens, replacing the
    initial pair of braces.  Press Ok.

* Choose "Diagram -> Add object graph node...".

  * Choose a node of central importance.  What is important will depend
    on the chosen input source code and the desired purpose of the
    diagram.  A `FunctionDecl` node might be a good choice.

* Resize the entity so you can see its name and contents.  Inside the
  main area you should see the attributes, then a blank line, then
  the pointers.

* Right-click the entity, choose "Follow pointer", then pick a pointer
  name.  If using a `FunctionDecl` as the first node, and it has
  parameters, `ParamInfo[0]` might be a good choice.

* Continue following pointers to explore the graph.  See the `ded`
  documentation on
  [Using ded to make diagrams of graphs](https://github.com/smcpeak/ded/doc/diagrams-of-graphs.md)
  for more details on how to do that.

# Licensing

This tool is offered under the same [Apache License](LICENSE.TXT) as
Clang itself.
