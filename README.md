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

For now, you have to edit the `Makefile` to set
`CLANG_LLVM_INSTALL_DIR` to point at a Clang installation directory.

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

# Importing into ded

After generating JSON, to import into `ded`:

* Start `ded`.

* Choose "Diagram -> Edit object graph..." from the menu.

  * Copy+paste the JSON text into the window that opens, replacing the
    initial pair of braces.  Press Ok.

* In `ded`, press "C" (Create) then left-click to create an entity.

* Double-click the newly created entity to edit it.

  * Outside `ded` (for example, in a text editor), look in the JSON
    output from `print-clang-ast.exe` for a node of interest.  A possible
    starting point is `TranslationUnitDecl 1`, representing the entire TU.

  * Type or paste the chosen node ID into the "Object graph node ID" box
    in the lower-right corner.

  * Enter `$(graphNodeID)` into the "Name" box.  (That means the node
    is named after its ID.  You could put something else instead or
    in addition.)

  * Enter `$(graphNodeShowFieldsAttrs)<newline>$(graphNodeShowFieldsPtrs)`
    into the "Attributes" box.  (Only the first node requires so much
    manual effort.)

  * Press Ok.

* Resize the entity so you can see its name and contents.  Inside the
  main area you should see the attributes, then a blank line, then
  the pointers.

* Right-click the entity, choose "Follow pointer", then pick a pointer
  name.  If using the TU as the first node, "LastDecl" might be a good
  choice.

* Continue following pointers to explore the graph.

* I'll add more documentation to `ded` soon explaining all the
  capabilities.

# Licensing

This tool is offered under the same [Apache License](LICENSE.TXT) as
Clang itself.
