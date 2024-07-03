// symbolic-line-mapper.cc
// Code for `symbolic-line-mapper.h`.

#include "symbolic-line-mapper.h"      // this module

#include "stringref-parse.h"           // StringRefParse

#include "smbase/map-util.h"           // mapFindOpt
#include "smbase/stringb.h"            // stringb
#include "smbase/xassert.h"            // xassert, xassertPrecondition

#include <map>                         // std::map


SymbolicLineMapper::LineToNameMap const &
SymbolicLineMapper::getLineToNameMap(clang::FileID fileID) const
{
  xassertPrecondition(fileID.isValid());

  if (auto itOpt = mapFindOpt(*m_fileIdToLineToName, fileID)) {
    // This should work, but it does not due to a Clang bug:
    // https://github.com/llvm/llvm-project/issues/96403
    //return (**itOpt).second;

    // Workaround:
    auto tmp = *itOpt;
    return (*tmp).second;
  }

  // Create an empty map for `fileID`.
  LineToNameMap &l2n = (*m_fileIdToLineToName)[fileID];

  // Get the source code for that file.
  std::optional<llvm::MemoryBufferRef> bufferOpt =
    m_srcMgr.getBufferOrNone(fileID);
  xassert(bufferOpt);

  // Scan the source, populating the map.
  StringRefParse parse(bufferOpt->getBuffer());
  while (parse.searchFor("SYMLINE(")) {
    std::string name = parse.getNextIdentifier();
    if (!name.empty()) {
      int line, col;
      parse.getLineCol(line, col);

      l2n.insert({line, name});
    }
  }

  return l2n;
}


SymbolicLineMapper::~SymbolicLineMapper()
{}


SymbolicLineMapper::SymbolicLineMapper(clang::ASTContext &astContext)
  : ClangUtil(astContext),
    m_fileIdToLineToName(new FileToLineToNameMap)
{}


std::string SymbolicLineMapper::symLineColStr(
  clang::SourceLocation loc) const
{
  std::string lineStr = symLineStr(loc);

  // This will yield 0 if `loc` is invalid, which is fine.
  int col = m_srcMgr.getPresumedColumnNumber(loc);

  return stringb(lineStr << ":" << col);
}


std::string SymbolicLineMapper::symLineStr(
  clang::SourceLocation loc) const
{
  if (loc.isInvalid()) {
    return "<invalid loc>";
  }

  clang::FileID fileID = m_srcMgr.getFileID(loc);
  if (fileID.isInvalid()) {
    // I don't think this can happen, but I'll just tolerate it.
    return "<invalid file ID>";
  }

  LineToNameMap const &l2n = getLineToNameMap(fileID);

  int line = m_srcMgr.getPresumedLineNumber(loc);

  if (auto itOpt = mapFindOpt(l2n, line)) {
    std::string const &name = (**itOpt).second;
    return name;
  }
  else {
    return stringb(line);
  }
}


// EOF
