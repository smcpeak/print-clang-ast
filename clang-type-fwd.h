// clang-type-fwd.h
// Forwards for clang/AST/Type.h.

#ifndef CLANG_TYPE_FWD_H
#define CLANG_TYPE_FWD_H

namespace clang {
  // This isn't everything in Type.h, just the classes I think are meant
  // to be part of the stable, public API.
  class Qualifiers;
  class QualType;
  class ExtQuals;
  class Type;
  // I'm skipping the Type subclasses for now.
  class TypeSourceInfo;
}

#endif // CLANG_TYPE_FWD_H
