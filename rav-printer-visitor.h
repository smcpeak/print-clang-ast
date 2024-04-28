// rav-printer-visitor.h
// Print AST using RecursiveASTVisitor.

#ifndef RAV_PRINTER_VISITOR_H
#define RAV_PRINTER_VISITOR_H

// this dir
#include "clang-ast-context-fwd.h"               // clang::ASTContext

// libc++
#include <iosfwd>                                // std::ostream


// Print the entire TU in 'astContext'.
void ravPrinterVisitorTU(std::ostream &os,
                         clang::ASTContext &astContext);


#endif // RAV_PRINTER_VISITOR_H
