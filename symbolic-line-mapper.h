// symbolic-line-mapper.h
// `SymbolicLineMapper`, which maps some line numbers to symbols (names).

#ifndef PCA_SYMBOLIC_LINE_MAPPER_H
#define PCA_SYMBOLIC_LINE_MAPPER_H

#include "symbolic-line-mapper-fwd.h"  // forwards for this module

#include "clang-ast-context-fwd.h"     // clang::ASTContext
#include "clang-source-location-fwd.h" // clang::{FileId, SourceLocation}
#include "clang-util.h"                // ClangUtil

#include "smbase/sm-unique-ptr.h"      // smbase::UniquePtr
#include "smbase/std-map-fwd.h"        // stdfwd::map
#include "smbase/std-optional-fwd.h"   // std::optional
#include "smbase/std-string-fwd.h"     // std::string

// TODO: Create a forward header for variant.
#include <variant>                     // std::variant


// Map source location lines to optional names.
class SymbolicLineMapper : public ClangUtil {
private:     // types
  // Map from line number to symbolic name.
  using LineToNameMap = stdfwd::map<int, std::string>;

  // Map from file ID to line->name map.
  using FileToLineToNameMap = stdfwd::map<clang::FileID, LineToNameMap>;

public:      // types
  typedef std::variant<std::string, int> StrOrInt;

private:     // data
  // Map from file ID to its line->name map.
  //
  // This is built on demand by scanning the associated source code when
  // the file ID is first used.
  //
  // If this were an embedded map, this would be `mutable` so that
  // `symLineColStr` can be `const`, thereby making `LineMapper const &`
  // a possible substitute for `ClangUtil const &`.  However,
  // `UniquePtr` does not propagate constness to the pointee, so it does
  // not actually have to be `mutable`.
  //
  smbase::UniquePtr<FileToLineToNameMap> m_fileIdToLineToName;

private:     // methods
  // Get the map for `fileID`, creating it first if necessary.
  LineToNameMap const &getLineToNameMap(clang::FileID fileID) const;

public:      // methods
  ~SymbolicLineMapper();

  SymbolicLineMapper(clang::ASTContext &astContext);

  // Get `loc` as `L:C`, where `L` is either a line number or a symbolic
  // name.  The latter is used when the line textually contains a string
  // of the form "SYMLINE(id)" where `id` is a C-like identifier.
  std::string symLineColStr(clang::SourceLocation loc) const;

  // Get just the line of `loc` as a name or number.
  std::string symLineStr(clang::SourceLocation loc) const;

  // If `loc` is an invalid location, return `nullopt`.  If `loc` is on
  // a line that has a symbolic name, return that name.  Otherwise,
  // return the line number as an integer.
  std::optional<StrOrInt> symLineStrOrIntOpt(
    clang::SourceLocation loc) const;

  // If `loc` is a valid location that has an associated symbolic line
  // number name, return that name.  Otherwise return `nullopt`.
  std::optional<std::string> symLineStrOpt(
    clang::SourceLocation loc) const;
};


// Defined in symbolic-line-mapper-test.cc.
void symbolic_line_mapper_unit_tests();


#endif // PCA_SYMBOLIC_LINE_MAPPER_H
