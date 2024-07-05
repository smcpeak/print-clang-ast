// macro-definition.h
// Define a macro that `macro-expansion.cc` will expand.

#ifndef MACRO_DEFINITION_H
#define MACRO_DEFINITION_H

void hiddenFunction();

#define visibleMacro() hiddenFunction() /*SYMLINE(macroDefnLine)*/

#endif // MACRO_DEFINITION_H
