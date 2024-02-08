// print-clang-ast-nodes.cc
// Code for print-clang-ast-nodes module.

#include "print-clang-ast-nodes-private.h"       // this module

#include "enum-util.h"                           // ENUM_TABLE_LOOKUP
#include "expose-template-common.h"              // clang::FunctionTemplateDecl_Common
#include "map-util.h"                            // mapFindOpt
#include "optional-util.h"                       // optionalToString
#include "spy-private.h"                         // ACCESS_PRIVATE_FIELD
#include "trace.h"                               // INIT_TRACE
#include "util.h"                                // doubleQuote, stringb

//#include "clang/AST/ASTDumper.h"                 // clang::ASTDumper
#include "clang/AST/DeclContextInternals.h"      // clang::StoredDeclsMap::size
#include "clang/AST/DeclFriend.h"                // clang::FriendDecl
#include "clang/AST/ExprCXX.h"                   // clang::CXXDependentScopeMemberExpr
#include "clang/Lex/Lexer.h"                     // clang::Lexer

#include "llvm/Support/raw_os_ostream.h"         // llvm::raw_os_ostream
#include "llvm/Support/raw_ostream.h"            // llvm::raw_string_ostream

#include <iterator>                              // std::distance
#include <iostream>                              // std::ostream
#include <string>                                // std::string

#include <assert.h>                              // assert


using clang::isa;
using clang::dyn_cast;

using std::string;


INIT_TRACE("print-clang-ast-nodes");


// Print a failed assertion message to the output, and yield false.
#define PRINT_ASSERT_FAIL(msg)              \
  (++m_failedAssertions,                    \
   (m_os << "\nAssertion failed: " << msg), \
   false)

#define PRINT_ASSERT_FAILED_NOPREFIX(msg) \
  (++m_failedAssertions,                  \
   (m_os << msg),                         \
   false)


// Check an assertion during AST printing.  If it fails, we emit the
// failure into the output and record that it happened, but do not stop.
// Yield the condition as a boolean so this can be used as the condition
// of an 'if' statement to only proceed if the assertion passes.
#define PRINT_ASSERT(cond)          \
  ((cond)?                          \
     (++m_passedAssertions, true) : \
     PRINT_ASSERT_FAIL(#cond))


// If 'node' is 'SubclassName', print it as such.
#define PRINT_IF_SUBCLASS(node, SubclassName)                    \
  if (auto subclassNode = dyn_cast<clang::SubclassName>(node)) { \
    print##SubclassName(subclassNode);                           \
  }


// Return JSON for an object with one key and value.
static std::string jsonObject1(
  std::string const &key1,
  std::string const &jsonValue1)
{
  return stringb("{ " <<
           doubleQuote(key1) << ": " << jsonValue1 <<
         " }");
}


// Return JSON for an object with two keys and values.
static std::string jsonObject2(
  std::string const &key1,
  std::string const &jsonValue1,
  std::string const &key2,
  std::string const &jsonValue2)
{
  return stringb("{ " <<
           doubleQuote(key1) << ": " << jsonValue1 << ", " <<
           doubleQuote(key2) << ": " << jsonValue2 <<
         " }");
}


// Print an attribute that has a value already expressed as JSON.
#define OUT_QATTR_JSON(qualifier, key, json)      \
  m_os << "  " << doubleQuote(stringb(ifLongForm( \
                    stringb(qualifier)) << key))  \
       << ": " << json << ",\n" /* user ; */

// Print an attribute that has a string value.
#define OUT_QATTR_STRING(qualifier, key, value)               \
  OUT_QATTR_JSON(qualifier, key, doubleQuote(stringb(value)))

// Print an attribute that has a pointer value.
#define OUT_QATTR_PTR(qualifier, key, id)                             \
  OUT_QATTR_JSON(qualifier, key, jsonObject1("ptr", doubleQuote(id)))

// Print an attribute that is a pointer to a Type.
#define OUT_QATTR_TYPE(qualifier, key, type) \
  OUT_QATTR_JSON(qualifier, key, typeIDSyntaxJson(type))

// Print an attribute that is a pointer to a statement.
#define OUT_QATTR_STMT(qualifier, key, stmt) \
  OUT_QATTR_PTR(qualifier, key, getStmtIDStr(stmt))

// Print an attribute that is a pointer to a declaration.
#define OUT_QATTR_DECL(qualifier, key, decl) \
  OUT_QATTR_PTR(qualifier, key, getDeclIDStr(decl))

// Print an attribute that has an integer value.
#define OUT_QATTR_INT(qualifier, key, value) \
  OUT_QATTR_JSON(qualifier, key, (value))

// Print an attribute that has a boolean value.
#define OUT_QATTR_BOOL(qualifier, key, value) \
  OUT_QATTR_JSON(qualifier, key, ((value)? "true" : "false"))

// Print an attribute that has a null value.
#define OUT_QATTR_NULL(qualifier, key)   \
  OUT_QATTR_JSON(qualifier, key, "null")

// Print an attribute that has a SourceLocation value.
#define OUT_QATTR_LOC(qualifier, key, loc)      \
  OUT_QATTR_STRING(qualifier, key, locStr(loc))

// Print an attribute that is a QualType.
#define OUT_QATTR_QUALTYPE(qualifier, key, qt)                \
    OUT_QATTR_JSON(qualifier, key, qualTypeIDSyntaxJson(qt))

// Print a placeholder for a value that I haven't implemented.
#define OUT_QATTR_TODO(qualifier, key)     \
  OUT_QATTR_STRING(qualifier, key, "TODO")

// Print an attribute whose value is a bit set, expressed as a
// space-prefixed sequence of words.  'trimWhitespace' removes the
// leading space when the value is not empty.
#define OUT_QATTR_BITSET(qualifier, key, value) \
  OUT_QATTR_STRING(qualifier, key, trimWhitespace(stringb(value)))


// Variants that do not take a qualifier parameter, and instead use an
// implicit qualifier that is empty.
#define OUT_ATTR_JSON(key, json)    OUT_QATTR_JSON("", key, json)
#define OUT_ATTR_STRING(key, value) OUT_QATTR_STRING("", key, value)
#define OUT_ATTR_PTR(key, id)       OUT_QATTR_PTR("", key, id)
#define OUT_ATTR_TYPE(key, type)    OUT_QATTR_TYPE("", key, type)
#define OUT_ATTR_STMT(key, stmt)    OUT_QATTR_STMT("", key, stmt)
#define OUT_ATTR_DECL(key, decl)    OUT_QATTR_DECL("", key, decl)
#define OUT_ATTR_INT(key, value)    OUT_QATTR_INT("", key, value)
#define OUT_ATTR_BOOL(key, value)   OUT_QATTR_BOOL("", key, value)
#define OUT_ATTR_NULL(key)          OUT_QATTR_NULL("", key)
#define OUT_ATTR_LOC(key, loc)      OUT_QATTR_LOC("", key, loc)
#define OUT_ATTR_QUALTYPE(key, qt)  OUT_QATTR_QUALTYPE("", key, qt)
#define OUT_ATTR_TODO(key)          OUT_QATTR_TODO("", key)
#define OUT_ATTR_BITSET(key, value) OUT_QATTR_BITSET("", key, value)


// Start a new object with identifier 'id'.
#define OUT_OBJECT(id) { \
  closeOpenObjectIf();   \
  openNewObject(id);     \
}


PrintClangASTNodes::PrintClangASTNodes(
  std::ostream &os,
  clang::ASTContext &astContext,
  PrintClangASTNodesConfiguration const &config,
  ClangASTNodeNumbering &numbering)

  : ClangUtil(astContext),
    m_os(os),
    m_config(config),
    m_numbering(numbering),
    m_mapCommonToFunctionTemplateDecl(),
    m_mapCommonToClassTemplateDecl(),
    m_passedAssertions(0),
    m_failedAssertions(0),
    m_objectIsOpen(false)
{}


PrintClangASTNodes::~PrintClangASTNodes()
{}


void PrintClangASTNodes::openNewObject(string const &id)
{
  m_os << "\n" << doubleQuote(id) << ": {\n";
  m_objectIsOpen = true;
}


void PrintClangASTNodes::closeOpenObjectIf()
{
  if (m_objectIsOpen) {
    m_os << "},\n";
    m_objectIsOpen = false;
  }
}


std::string PrintClangASTNodes::shortAndLongForms(
  std::string const &shortForm,
  std::string const &longForm) const
{
  if (m_config.m_printQualifiers) {
    return longForm;
  }
  else {
    return shortForm;
  }
}


std::string PrintClangASTNodes::ifLongForm(
  std::string const &qualifier) const
{
  return shortAndLongForms("", qualifier);
}


namespace {
  // Just a uniquely-named class, never defined.
  class DeclSpyHelper;
  class RedeclarableTemplateDeclSpyHelper;
}

namespace clang {
  /*
    In this section, we subvert C++ access control by impersonating a
    class that is a friend of some target AST class in order to spy on
    it.  When I use a concrete class, this obviously violates the one
    definition rule (ODR), but shouldn't cause problems in practice
    since all the methods have non-colliding names and the real class is
    not defined in this TU.  However, when the spy target befriends a
    template, I can use an explicit specialization instead, which should
    make that instance of the trick ODR-compliant.

    There is a question on SO about how to do something like this:

      https://stackoverflow.com/questions/424104/can-i-access-private-members-from-outside-the-class-without-using-friends

    and a fairly general solution presented there is to use a trick
    involving pointer to member (PTM) and a language loophole for
    explicit instantiation.  But that won't work here because many of
    the fields I want to access are bitfield members, and PTM does not
    work for them.
  */

  #define DEFINE_FIELD_GETTER(className, fieldName)  \
    static auto fieldName(clang::className const *p) \
      { return p->fieldName; }
  #define DEFINE_METHOD_GETTER(className, methodName) \
    static auto methodName(clang::className const *p) \
      { return p->methodName(); }
  #define DEFINE_EXPR_GETTER(className, getterName, expr) \
    static auto getterName(clang::className const *p) \
      { return p->expr; }

  // Return something stored in the "Bits" bitfield.
  #define DEFINE_BITS_GETTER(className, bitsName)   \
    static auto bitsName(clang::className const *p) \
      { return p->className##Bits.bitsName; }

  // Variant for when the method is not marked 'const'.  One example is
  // the method might load a lazy pointer, which I consider to still be
  // within the spirit of what 'const' conveys, and anyway there will
  // not be any lazy pointers since I'm only using this on parsed (not
  // loaded) ASTs.
  #define DEFINE_CONST_CAST_METHOD_GETTER(className, methodName) \
    static auto methodName(clang::className const *p) \
      { return const_cast<clang::className*>(p)->methodName(); }

  // Concrete class, technical ODR violation.
  #define DeclContextSpy ASTWriter
  class DeclContextSpy {
  public:
    // Although I could define a getter to return DeclContextBitfields
    // as a whole, I would not be able to make use of it since all of
    // its members are private, and its only friend is DeclContext,
    // which I cannot impersonate since it's already defined in this TU.
    // Fortunately, all the flags have corresponding getters (some of
    // which are private, so the spy is still needed).
    DEFINE_METHOD_GETTER(DeclContext, hasExternalLexicalStorage)
    DEFINE_METHOD_GETTER(DeclContext, hasExternalVisibleStorage)
    DEFINE_METHOD_GETTER(DeclContext, hasNeedToReconcileExternalVisibleStorage)
    DEFINE_METHOD_GETTER(DeclContext, hasLazyLocalLexicalLookups)
    DEFINE_METHOD_GETTER(DeclContext, hasLazyExternalLexicalLookups)
    DEFINE_METHOD_GETTER(DeclContext, shouldUseQualifiedLookup)

    DEFINE_FIELD_GETTER(DeclContext, FirstDecl);
    DEFINE_FIELD_GETTER(DeclContext, LastDecl);
  };

  #define DeclSpy Redeclarable<DeclSpyHelper>
  template <>
  class DeclSpy {
  public:
    DEFINE_FIELD_GETTER(Decl, InvalidDecl)
    DEFINE_FIELD_GETTER(Decl, HasAttrs)
    DEFINE_FIELD_GETTER(Decl, Implicit)
    DEFINE_FIELD_GETTER(Decl, Used)
    DEFINE_FIELD_GETTER(Decl, Referenced)
    DEFINE_FIELD_GETTER(Decl, TopLevelDeclInObjCContainer)
    DEFINE_FIELD_GETTER(Decl, FromASTFile)
    DEFINE_FIELD_GETTER(Decl, IdentifierNamespace)

    DEFINE_METHOD_GETTER(Decl, hasCachedLinkage)
    DEFINE_METHOD_GETTER(Decl, getCachedLinkage)
  };

  #define TemplateParameterListSpy FixedSizeTemplateParameterListStorage<999, false>
  template <>
  class TemplateParameterListSpy {
  public:
    DEFINE_FIELD_GETTER(TemplateParameterList, ContainsUnexpandedParameterPack)
    DEFINE_FIELD_GETTER(TemplateParameterList, HasRequiresClause)
    DEFINE_FIELD_GETTER(TemplateParameterList, HasConstrainedParameters)
  };

  // This class doesn't actually exist anywhere in clang, even though
  // RedeclarableTemplateDecl befriends it.  I suspect that just
  // 'Redeclarable' is what was intended, but the friendship turned out
  // not to be needed, so the erroneous declaration went unnoticed.  It
  // doesn't matter to me what the spy is called, though, so I'll define
  // the erroneously-named template anyway, then specialize it.
  template <class T>
  class RedeclarableTemplate {};

  #define RedeclarableTemplateDeclSpy RedeclarableTemplate<RedeclarableTemplateDeclSpyHelper>
  template <>
  class RedeclarableTemplateDeclSpy {
  public:
    DEFINE_FIELD_GETTER(RedeclarableTemplateDecl, Common)

    DEFINE_FIELD_GETTER(RedeclarableTemplateDecl::CommonBase, InstantiatedFromMember)
    DEFINE_FIELD_GETTER(RedeclarableTemplateDecl::CommonBase, LazySpecializations)
    DEFINE_FIELD_GETTER(RedeclarableTemplateDecl::CommonBase, InjectedArgs)
  };

  // Use ASTDeclReader for several Decl subtypes.
  // Columns: \S+ \S+ \S+
  #define TemplateTypeParmDeclSpy                   ASTDeclReader
  #define VarDeclSpy                                ASTDeclReader
  #define DeclaratorDeclSpy                         ASTDeclReader
  #define ParmVarDeclSpy                            ASTDeclReader
  #define FunctionDeclSpy                           ASTDeclReader
  #define FunctionTemplateDeclSpy                   ASTDeclReader
  #define ClassTemplateDeclSpy                      ASTDeclReader
  #define TagDeclSpy                                ASTDeclReader
  #define CXXRecordDeclSpy                          ASTDeclReader
  #define FieldDeclSpy                              ASTDeclReader
  #define ClassTemplateSpecializationDeclSpy        ASTDeclReader
  #define ClassTemplatePartialSpecializationDeclSpy ASTDeclReader
  #define CXXConstructorDeclSpy                     ASTDeclReader
  #define FriendDeclSpy                             ASTDeclReader
  class TemplateTypeParmDeclSpy {
  public:
    DEFINE_FIELD_GETTER(TemplateTypeParmDecl, Typename)
    DEFINE_FIELD_GETTER(TemplateTypeParmDecl, HasTypeConstraint)
    DEFINE_FIELD_GETTER(TemplateTypeParmDecl, TypeConstraintInitialized)
    DEFINE_FIELD_GETTER(TemplateTypeParmDecl, ExpandedParameterPack)
    DEFINE_FIELD_GETTER(TemplateTypeParmDecl, NumExpanded)

    DEFINE_FIELD_GETTER(VarDecl, Init);

    DEFINE_METHOD_GETTER(DeclaratorDecl, hasExtInfo)

    static unsigned DefaultArgKind(clang::ParmVarDecl const *decl)
      { return decl->ParmVarDeclBits.DefaultArgKind; }
    static unsigned ParameterIndex(clang::ParmVarDecl const *decl)
      { return decl->ParmVarDeclBits.ParameterIndex; }

    // Stringify a VarDecl::DefaultArgKind.
    //
    // This is not in clang-util because the type it decodes is not
    // public, and I want to confine my access control shenanigans to
    // this file.
    static std::string defaultArgKindStr(unsigned kind);

    DEFINE_METHOD_GETTER(FunctionDecl, isDeletedBit)
    DEFINE_METHOD_GETTER(FunctionDecl, hasODRHash)

    DEFINE_FIELD_GETTER(FunctionDecl, Body)
    DEFINE_FIELD_GETTER(FunctionDecl, ODRHash)
    DEFINE_FIELD_GETTER(FunctionDecl, EndRangeLoc)
    DEFINE_FIELD_GETTER(FunctionDecl, DefaultKWLoc)

    DEFINE_METHOD_GETTER(FunctionTemplateDecl, getCommonPtr)

    DEFINE_METHOD_GETTER(ClassTemplateDecl, getCommonPtr)

    DEFINE_FIELD_GETTER(TagDecl, TypedefNameDeclOrQualifier)

    using CXXRecordDecl_DefinitionData =
      struct CXXRecordDecl::DefinitionData;
    using CXXRecordDecl_LambdaDefinitionData =
      CXXRecordDecl::LambdaDefinitionData;

    DEFINE_FIELD_GETTER(CXXRecordDecl, DefinitionData)
    DEFINE_FIELD_GETTER(CXXRecordDecl, TemplateOrInstantiation)

    // Stringify a CXXRecordDecl::SpecialMemberFlags.
    static std::string specialMemberFlagsStr(unsigned flags);

    DEFINE_FIELD_GETTER(FieldDecl, CachedFieldIndex)

    DEFINE_FIELD_GETTER(ClassTemplateSpecializationDecl, SpecializedTemplate);
    DEFINE_FIELD_GETTER(ClassTemplateSpecializationDecl, ExplicitInfo);

    using ClassTemplateSpecializationDecl_SpecializedPartialSpecialization =
      ClassTemplateSpecializationDecl::SpecializedPartialSpecialization;
    using ClassTemplateSpecializationDecl_ExplicitSpecializationInfo =
      ClassTemplateSpecializationDecl::ExplicitSpecializationInfo;

    DEFINE_FIELD_GETTER(ClassTemplatePartialSpecializationDecl, InstantiatedFromMember);

    DEFINE_EXPR_GETTER(CXXConstructorDecl, HasTrailingExplicitSpecifier,
      numTrailingObjects(clang::ASTConstraintSatisfaction::
        OverloadToken<clang::ExplicitSpecifier>()))

    DEFINE_CONST_CAST_METHOD_GETTER(FriendDecl, getNextFriend)
  };

  #define DeclRefExprSpy                 ASTStmtReader
  #define MemberExprSpy                  ASTStmtReader
  #define CXXDependentScopeMemberExprSpy ASTStmtReader
  class DeclRefExprSpy {
  public:
    DEFINE_METHOD_GETTER(DeclRefExpr, hasFoundDecl)

    DEFINE_BITS_GETTER(MemberExpr, IsArrow)
    DEFINE_BITS_GETTER(MemberExpr, HasQualifierOrFoundDecl)
    DEFINE_BITS_GETTER(MemberExpr, HasTemplateKWAndArgsInfo)
    DEFINE_BITS_GETTER(MemberExpr, HadMultipleCandidates)

    DEFINE_FIELD_GETTER(CXXDependentScopeMemberExpr, Base)
  };

  #define InjectedClassNameTypeSpy ASTReader
  #define TagTypeSpy               ASTReader
  class InjectedClassNameTypeSpy {
  public:
    DEFINE_FIELD_GETTER(InjectedClassNameType, Decl)

    DEFINE_FIELD_GETTER(TagType, decl)
  };


  #undef DEFINE_FIELD_GETTER
  #undef DEFINE_METHOD_GETTER


  /*static*/ std::string VarDeclSpy::defaultArgKindStr(unsigned kind)
  {
    ENUM_TABLE_LOOKUP_OR_STRINGB_CAST(
      clang::VarDecl::, DefaultArgKind, kind,

      DAK_None,
      DAK_Unparsed,
      DAK_Uninstantiated,
      DAK_Normal,
    )
  }

  /*static*/ std::string CXXRecordDeclSpy::specialMemberFlagsStr(
    unsigned flags)
  {
    // There is not actually an 'SMF_None' enumerator, but I think there
    // should be, and anyway it's more evocative here.
    BITFLAGS_TABLE_LOOKUP(clang::CXXRecordDecl::,
      "SpecialMemberFlags", "SMF_None", flags,

      SMF_DefaultConstructor,
      SMF_CopyConstructor,
      SMF_MoveConstructor,
      SMF_CopyAssignment,
      SMF_MoveAssignment,
      SMF_Destructor,
    )
  }

  #if 0  // not needed
  /*static*/ std::string FieldDeclSpy::initStorageKindStr(
    clang::FieldDecl::InitStorageKind isk)
  {
    ENUM_TABLE_LOOKUP_OR_STRINGB_CAST(
      clang::FieldDecl::, InitStorageKind, isk,

      ISK_NoInit,
      ISK_InClassCopyInit,
      ISK_InClassListInit,
      ISK_CapturedVLAType,
    )
  }
  #endif // 0
} // namespace clang


// Read a field or call a method via the spy class.
#define SPY(className, ptr, memberName) \
  clang::className##Spy::memberName(ptr)


// I need a typedef for use as a macro argument, since otherwise the
// commas interfere with macro argument parsing.
typedef llvm::PointerIntPair<clang::FunctionDecl *, 1, bool>
  PointerIntPair_FunctionDecl_1_bool;

// Activate spying using the other method, which is necessary when the
// target does not declare any friends we can impersonate.
ENABLE_ACCESS_PRIVATE_FIELD(
  clang::FunctionTemplateSpecializationInfo,
  PointerIntPair_FunctionDecl_1_bool,
  Function,
  0 /*discrim*/);


// Convert a pointer to the fake DD to pointer to the real DD.
static clang::CXXRecordDeclSpy::CXXRecordDecl_DefinitionData const *toRealDD(
  clang::Fake_CXXRecordDecl_DefinitionData const *dd)
{
  return reinterpret_cast<
    clang::CXXRecordDeclSpy::CXXRecordDecl_DefinitionData const *>(dd);
}

// And back.
static clang::Fake_CXXRecordDecl_DefinitionData *toFakeDD(
  clang::CXXRecordDeclSpy::CXXRecordDecl_DefinitionData *dd)
{
  return reinterpret_cast<
    clang::Fake_CXXRecordDecl_DefinitionData *>(dd);
}


// ----------------- PrintClangASTNodes: declarations ------------------
void PrintClangASTNodes::printDeclContext(
  clang::DeclContext const *declContext)
{
  // The naming conventions are not consistent, so I need both the
  // method name and flag name, since ultimately what I want to print is
  // the actual flag name.
  #define DECLCONTEXT_FLAG(methodName, flagName) \
    (SPY(DeclContext, declContext, methodName)? " " #flagName : "")

  // DeclContextBitfields also stores the DeclKind in order to allow
  // dyn_cast from DeclContext to Decl, since that requires knowing
  // the most-derived type.  It should, of course, agree with
  // Decl::DeclKind.
  OUT_QATTR_STRING("DeclContext::", "DeclKind",
    declContext->getDeclKindName() << "Decl");

  OUT_QATTR_BITSET("DeclContext::", "DeclContextBits",
    DECLCONTEXT_FLAG(hasExternalLexicalStorage,
                        ExternalLexicalStorage) <<
    DECLCONTEXT_FLAG(hasExternalVisibleStorage,
                        ExternalVisibleStorage) <<
    DECLCONTEXT_FLAG(hasNeedToReconcileExternalVisibleStorage,
                        NeedToReconcileExternalVisibleStorage) <<
    DECLCONTEXT_FLAG(hasLazyLocalLexicalLookups,
                     HasLazyLocalLexicalLookups) <<
    DECLCONTEXT_FLAG(hasLazyExternalLexicalLookups,
                     HasLazyExternalLexicalLookups) <<
    DECLCONTEXT_FLAG(shouldUseQualifiedLookup,
                           UseQualifiedLookup));

  OUT_QATTR_STRING("DeclContext::", "LookupPtr",
    (declContext->getLookupPtr()?
      stringb("StoredDeclsMap with " <<
              declContext->getLookupPtr()->size() << " entries") :
      string("null")));

  OUT_QATTR_PTR("DeclContext::", "FirstDecl",
    getOrCreateDeclIDStr(SPY(DeclContext, declContext, FirstDecl)));
  OUT_QATTR_PTR("DeclContext::", "LastDecl",
    getOrCreateDeclIDStr(SPY(DeclContext, declContext, LastDecl)));

  #undef DECLCONTEXT_FLAG
}


void PrintClangASTNodes::printTemplateParameterList(
  std::string const &qualifier,
  std::string const &label,
  clang::TemplateParameterList const * NULLABLE params)
{
  if (!params) {
    // At the moment, I do not know of a specific case that would
    // trigger this, but I want to be defensive generally while
    // printing.
    OUT_QATTR_NULL(qualifier, label);
    return;
  }

  #define TEMPLATEPARAMETERLIST_FLAG(flagName) \
    (SPY(TemplateParameterList, params, flagName)? " " #flagName : "")

  OUT_QATTR_STRING(qualifier, label,
    doubleQuote(templateParameterListStr(params)));

  OUT_QATTR_STRING(qualifier, label << "->TemplateLoc",
    locStr(params->getTemplateLoc()));

  OUT_QATTR_STRING(qualifier, label << "->LAngleLoc",
    locStr(params->getLAngleLoc()));

  OUT_QATTR_STRING(qualifier, label << "->RAngleLoc",
    locStr(params->getRAngleLoc()));

  OUT_QATTR_STRING(qualifier, label << "->NumTParams",
    params->size());

  OUT_QATTR_BITSET(qualifier, label << "->Flags",
    TEMPLATEPARAMETERLIST_FLAG(ContainsUnexpandedParameterPack) <<
    TEMPLATEPARAMETERLIST_FLAG(HasRequiresClause) <<
    TEMPLATEPARAMETERLIST_FLAG(HasConstrainedParameters));

  for (unsigned i=0; i < params->size(); ++i) {
    OUT_QATTR_PTR(qualifier << label << "->", "Param[" << i << "]",
      getDeclIDStr(params->getParam(i)));
  }

  OUT_QATTR_PTR(qualifier, label << "->Requires",
    getStmtIDStr(params->getRequiresClause()));

  #undef TEMPLATEPARAMETERLIST_FLAG
}


void PrintClangASTNodes::printTemplateArgumentList(
  std::string const &qualifier,
  std::string const &label,
  clang::TemplateArgumentList const &args)
{
  for (unsigned i=0; i < args.size(); ++i) {
    clang::TemplateArgument const &arg = args[i];

    printTemplateArgument(qualifier,
      stringb(label << "[" << i << "]"), arg);
  }
}


void PrintClangASTNodes::printTemplateArgumentListInfo(
  std::string const &qualifier,
  std::string const &label,
  clang::TemplateArgumentListInfo const &args)
{
  OUT_QATTR_LOC(qualifier, label << ".LAngleLoc",
    args.getLAngleLoc());
  OUT_QATTR_LOC(qualifier, label << ".RAngleLoc",
    args.getRAngleLoc());

  for (unsigned i=0; i < args.size(); ++i) {
    clang::TemplateArgumentLoc const &arg = args[i];

    printTemplateArgumentLoc(qualifier,
      stringb(label << "[" << i << "]"), &arg);
  }
}


void PrintClangASTNodes::printASTTemplateArgumentListInfo(
  std::string const &qualifier,
  std::string const &label,
  clang::ASTTemplateArgumentListInfo const * NULLABLE args)
{
  if (!args) {
    OUT_QATTR_NULL(qualifier, label);
    return;
  }

  OUT_QATTR_LOC(qualifier, label << ".LAngleLoc",
    args->LAngleLoc);
  OUT_QATTR_LOC(qualifier, label << ".RAngleLoc",
    args->RAngleLoc);

  for (unsigned i=0; i < args->NumTemplateArgs; ++i) {
    clang::TemplateArgumentLoc const &arg = (*args)[i];

    printTemplateArgumentLoc(qualifier,
      stringb(label << "[" << i << "]"), &arg);
  }
}


void PrintClangASTNodes::printTemplateArgument(
  std::string const &qualifier,
  std::string const &label,
  clang::TemplateArgument const &arg)
{
  string tovv = shortAndLongForms(".TOV.V", ".TypeOrValue.V");

  OUT_QATTR_STRING(qualifier, label << ".Kind",
    templateArgumentKindStr(arg.getKind()));

  IF_CLANG_17(
    OUT_QATTR_STRING(qualifier, label << ".IsDefaulted",
      arg.getIsDefaulted())
    ,
    /* 'IsDefaulted' does not exist in Clang 16. */);

  switch (arg.getKind()) {
    case clang::TemplateArgument::Null:
      // Nothing more to print.
      break;

    case clang::TemplateArgument::Type:
      OUT_QATTR_QUALTYPE(qualifier, label << tovv,
        arg.getAsType());
      break;

    case clang::TemplateArgument::Declaration:
      OUT_QATTR_QUALTYPE(qualifier, label << ".DeclArg.QT",
        arg.getParamTypeForDecl());
      OUT_QATTR_PTR(qualifier, label << ".DeclArg.D",
        getDeclIDStr(arg.getAsDecl()));
      break;

    case clang::TemplateArgument::NullPtr:
      OUT_QATTR_QUALTYPE(qualifier, label << tovv,
        arg.getNullPtrType());
      break;

    case clang::TemplateArgument::Integral: {
      llvm::APSInt value = arg.getAsIntegral();

      // The 'BitWidth' and 'IsUnsigned' fields are not directly
      // accessible, but get copied into the 'value', and seemingly
      // stored there without further alteration, so I should be able
      // to read them from there.
      OUT_QATTR_STRING(qualifier, label << ".Integer.BitWidth",
        value.getBitWidth());
      OUT_QATTR_STRING(qualifier, label << ".Integer.IsUnsigned",
        value.isUnsigned());

      OUT_QATTR_QUALTYPE(qualifier, label << ".Integer.Type",
        arg.getIntegralType());
      OUT_QATTR_STRING(qualifier, label << ".Integer.Value",
        apsIntStr(value));
      break;
    }

    case clang::TemplateArgument::TemplateExpansion:
      OUT_QATTR_STRING(qualifier, label << ".TemplateArg.NumExpansions",
        optionalToString(arg.getNumTemplateExpansions(), "absent"));
      // Fallthrough.

    case clang::TemplateArgument::Template:
      printTemplateName(
        qualifier,
        stringb(label << ".TemplateArg.Name"),
        arg.getAsTemplateOrTemplatePattern());
      break;

    case clang::TemplateArgument::Expression:
      OUT_QATTR_PTR(qualifier, label << tovv,
        getStmtIDStr(arg.getAsExpr()));
      break;

    case clang::TemplateArgument::Pack: {
      OUT_QATTR_STRING(qualifier, label << ".Args.NumArgs",
        arg.pack_size());

      unsigned i = 0;
      for (clang::TemplateArgument const &packArg : arg.pack_elements()) {
        printTemplateArgument(
          qualifier,
          stringb(label << ".Args.Args[" << i++ << "]"),
          packArg);
      }

      break;
    }

    // No default because I want a warning on a missing case.
  }
}


void PrintClangASTNodes::printTemplateName(
  std::string const &qualifier,
  std::string const &label,
  clang::TemplateName const &tname)
{
  // 'getKind()' returns a bogus value for a null name, so we need to
  // check this separately.
  if (tname.isNull()) {
    OUT_QATTR_STRING(qualifier, label, "null");
    return;
  }

  OUT_QATTR_STRING(qualifier, label << ".getKind()",
    templateNameKindStr(tname.getKind()));

  switch (tname.getKind()) {
    case clang::TemplateName::Template:
      OUT_QATTR_PTR(qualifier, label << ".Storage.Decl",
        getDeclIDStr(tname.getAsTemplateDecl()));
      break;

    case clang::TemplateName::OverloadedTemplate:
    case clang::TemplateName::AssumedTemplate:
    case clang::TemplateName::SubstTemplateTemplateParm:
    case clang::TemplateName::SubstTemplateTemplateParmPack:
    case clang::TemplateName::UsingTemplate:
      OUT_QATTR_STRING(qualifier, label << ".Storage.Uncommon", "TODO");
      break;

    case clang::TemplateName::QualifiedTemplate:
      OUT_QATTR_STRING(qualifier, label << ".Storage.Qualified", "TODO");
      break;

    case clang::TemplateName::DependentTemplate:
      OUT_QATTR_STRING(qualifier, label << ".Storage.Dependent", "TODO");
      break;

    // No default so we get a warning if a case is missing.
  }
}


void PrintClangASTNodes::printTemplateArgumentLoc(
  std::string const &qualifier,
  std::string const &label,
  clang::TemplateArgumentLoc const *targLoc)
{
  OUT_QATTR_STRING(qualifier, label << "::Argument",
    doubleQuote(templateArgumentStr(targLoc->getArgument())));

  clang::TemplateArgumentLocInfo locInfo = targLoc->getLocInfo();

  // At least when using the public API, it is not possible to know
  // which kind of thing is stored in 'locInfo' without looking at the
  // template argument.  The case logic here follows the checks and
  // assertions in the 'TemplateArgumentLoc' accessor methods.
  switch (targLoc->getArgument().getKind()) {
    case clang::TemplateArgument::Type:
      OUT_QATTR_STRING(qualifier, label << "::LocInfo::TypeSourceInfo",
        typeSourceInfoStr(locInfo.getAsTypeSourceInfo()));
      break;

    case clang::TemplateArgument::Expression:
    case clang::TemplateArgument::Declaration:
    case clang::TemplateArgument::NullPtr:
    case clang::TemplateArgument::Integral:
      OUT_QATTR_PTR(qualifier, label << "::LocInfo::Expr",
        getStmtIDStr(locInfo.getAsExpr()));
      break;

    case clang::TemplateArgument::Template:
    case clang::TemplateArgument::TemplateExpansion:
      OUT_QATTR_STRING(qualifier, label << "::LocInfo::TemplateTemplateArgLocInfo::Qualifier",
         nestedNameSpecifierLocStr(locInfo.getTemplateQualifierLoc()));
      OUT_QATTR_STRING(qualifier, label << "::LocInfo::TemplateTemplateArgLocInfo::TemplateNameLoc",
         locStr(locInfo.getTemplateNameLoc()));

      if (targLoc->getArgument().getKind() ==
            clang::TemplateArgument::TemplateExpansion) {
        OUT_QATTR_STRING(qualifier, label << "::LocInfo::TemplateTemplateArgLocInfo::EllipsisLoc",
          locStr(locInfo.getTemplateEllipsisLoc()));
      }

      break;

    default:
      OUT_QATTR_STRING(qualifier, label << "::LocInfo", "(none)");
      break;
  }
}


void PrintClangASTNodes::printDeclarationNameInfoLocInfo(
  std::string const &qualifier,
  std::string const &label,
  clang::DeclarationNameInfo dni)
{
  // Each of these will be null or empty if 'dni' stores a different
  // kind of thing.  It can be (and in fact usually is) none of these
  // three.
  clang::TypeSourceInfo *dniTSI = dni.getNamedTypeInfo();
  clang::SourceRange dniRange = dni.getCXXOperatorNameRange();
  clang::SourceLocation dniLoc = dni.getCXXLiteralOperatorNameLoc();

  OUT_QATTR_STRING(qualifier, label,
    (dniTSI?             typeSourceInfoStr(dniTSI) :
     dniRange.isValid()? sourceRangeStr(dniRange)  :
     dniLoc.isValid()?   locStr(dniLoc)            :
                         string("(none)")          ));
}


void PrintClangASTNodes::printDeclarationNameInfo(
  std::string const &qualifier,
  std::string const &label,
  clang::DeclarationNameInfo dni)
{
  OUT_QATTR_STRING(qualifier, label << ".Name",
    declarationNameAndKindStr(dni.getName()));

  OUT_QATTR_LOC(qualifier, label << ".NameLoc",
    dni.getLoc());

  printDeclarationNameInfoLocInfo(qualifier,
    stringb(label << ".LocInfo"),
      dni);
}


void PrintClangASTNodes::printDeclGroupRef(
  std::string const &qualifier,
  std::string const &label,
  clang::DeclGroupRef dgr)
{
  if (dgr.isNull()) {
    OUT_QATTR_NULL(qualifier, label);
  }
  else if (dgr.isSingleDecl()) {
    OUT_QATTR_PTR(qualifier, label,
      getDeclIDStr(dgr.getSingleDecl()));
  }
  else {
    unsigned i = 0;
    for (auto it = dgr.begin(); it != dgr.end(); ++it) {
      OUT_QATTR_PTR(qualifier, label << "[" << (i++) << "]",
        getDeclIDStr(*it));
    }
  }
}


void PrintClangASTNodes::printCXXCtorInitializer(
  std::string const &qualifier,
  std::string const &label,
  clang::CXXCtorInitializer const *init)
{
  // Initializee.
  if (clang::TypeSourceInfo const *tsi = init->getTypeSourceInfo()) {
    OUT_QATTR_STRING(qualifier,
      label << ".Initializee" << ifLongForm(".TypeSourceInfo"),
        typeSourceInfoStr(tsi));
  }
  else if (clang::FieldDecl const *fieldDecl = init->getMember()) {
    OUT_QATTR_PTR(qualifier,
      label << ".Initializee" << ifLongForm(".FieldDecl"),
        getDeclIDStr(fieldDecl));
  }
  else if (clang::IndirectFieldDecl const *ifd = init->getIndirectMember()) {
    OUT_QATTR_PTR(qualifier,
      label << ".Initializee" << ifLongForm(".IndirectFieldDecl"),
        getDeclIDStr(fieldDecl));
  }
  else {
    // Should not happen.
    OUT_QATTR_STRING(qualifier,
      label << ".Initializee",
        "unknown?");
  }

  OUT_QATTR_PTR(qualifier, label << ".Init",
    getStmtIDStr(init->getInit()));

  OUT_QATTR_LOC(qualifier, label << ".MemberOrEllipsisLocation",
    init->getMemberLocation());
  OUT_QATTR_LOC(qualifier, label << ".LParenLoc",
    init->getLParenLoc());
  OUT_QATTR_LOC(qualifier, label << ".RParenLoc",
    init->getRParenLoc());

  OUT_QATTR_BITSET(qualifier, label << ".Flags",
    (init->isDelegatingInitializer()?
         " IsDelegating": "") <<
    (init->isBaseInitializer() && init->isBaseVirtual()?
                                          " IsVirtual" : "") <<
    (init->isWritten()?
         " IsWritten" : ""));

  OUT_QATTR_INT(qualifier, label << ".SourceOrder",
    init->getSourceOrder());
}


void PrintClangASTNodes::printCXXBaseSpecifier(
  std::string const &qualifier,
  std::string const &label,
  clang::CXXBaseSpecifier const *bspec)
{
  OUT_QATTR_STRING(qualifier, label << ".Range",
    sourceRangeStr(bspec->getSourceRange()));

  OUT_QATTR_LOC(qualifier, label << ".EllipsisLoc",
    bspec->getEllipsisLoc());

  OUT_QATTR_BITSET(qualifier, label << ".Flags",
    accessSpecifierStr(bspec->getAccessSpecifierAsWritten()) <<
    (bspec->isVirtual()?
            " Virtual" : "") <<
    (bspec->isBaseOfClass()?
            " BaseOfClass" : "") <<
    (bspec->getInheritConstructors()?
             " InheritConstructors" : ""));

  OUT_QATTR_STRING(qualifier, label << ".BaseTypeInfo",
    typeSourceInfoStr(bspec->getTypeSourceInfo()));
}


std::string PrintClangASTNodes::ptrAndPreview(
  std::string const &ptrVal,
  std::string const &previewVal)
{
  return jsonObject2(
    "ptr", doubleQuote(ptrVal),
    "preview", doubleQuote(previewVal)
  );
}


std::string PrintClangASTNodes::typeIDSyntaxJson(
  clang::Type const * NULLABLE type)
{
  if (type) {
    return ptrAndPreview(
      getOrCreateTypeIDStr(type),
      typeStr(type)
    );
  }
  else {
    return "null";
  }
}


std::string PrintClangASTNodes::qualTypeIDSyntaxJson(
  clang::QualType qualType)
{
  if (qualType.isNull()) {
    return "null";
  }
  else {
    clang::Type const *type = qualType.getTypePtr();
    assert(type);

    return ptrAndPreview(
      getOrCreateTypeIDStr(type),
      qualTypeStr(qualType)
    );
  }
}


std::string PrintClangASTNodes::nestedNameSpecifierIDSyntaxJson(
  clang::NestedNameSpecifier const * NULLABLE nns)
{
  if (nns) {
    return ptrAndPreview(
      getNestedNameSpecifierIDStr(nns),
      nestedNameSpecifierStr_nq(nns)
    );
  }
  else {
    return "null";
  }
}


std::string PrintClangASTNodes::getNextToken(
  clang::SourceLocation /*IN/OUT*/ &loc) const
{
  std::optional<clang::Token> tok =
    clang::Lexer::findNextToken(loc, m_srcMgr, getLangOptions());
  if (tok) {
    std::string spelling =
      clang::Lexer::getSpelling(*tok, m_srcMgr, getLangOptions());

    TRACE3("getNextToken:"
           " loc=" << locStr(tok->getLocation()) <<
           " len=" << tok->getLength() <<
           " spelling=" << doubleQuote(spelling) <<
           "");

    assert(tok->getLocation() != loc);
    assert(tok->getLength() >= 1);
    loc = tok->getLocation().getLocWithOffset(tok->getLength() - 1);
    return spelling;
  }
  else {
    return "";
  }
}


clang::SourceLocation PrintClangASTNodes::getEndOfEqDelete(
  clang::FunctionDecl const *decl)
{
  clang::SourceLocation cursor = decl->getEndLoc();
  if (!cursor.isValid()) {
    return cursor;
  }

  // We cannot use clang::Lexer::findLocationAfterToken to recognize
  // 'kw_delete' because the lexer is being used in "raw" mode, so it
  // always returns 'raw_identifier' as the token kind for identifiers.

  if (getNextToken(cursor /*IN/OUT*/) != "=") {
    return clang::SourceLocation();
  }

  if (getNextToken(cursor /*IN/OUT*/) != "delete") {
    return clang::SourceLocation();
  }

  return cursor;
}


template <class T>
void PrintClangASTNodes::printRedeclarable(
  clang::Redeclarable<T> const *decl)
{
  OUT_ATTR_PTR(shortAndLongForms("RD::Prev", "Redeclarable::Previous"),
    // The logic here looks weird, but it really is, in the end,
    // just reading a single pointer value, the same value in both
    // branches of the conditional.
    getDeclIDStr(decl->isFirstDecl()?
                   decl->getMostRecentDecl() :
                   decl->getPreviousDecl()));

  OUT_ATTR_PTR(shortAndLongForms("RD::First", "Redeclarable::First"),
    getDeclIDStr(decl->getFirstDecl()));

  // There isn't actually a method called 'redecls_size', but I will
  // pretend there is.  Since it is usually 1, and only interesting
  // when not 1, I'll suppress the case of 1 in abbreviated mode.
  int size = std::distance(decl->redecls_begin(), decl->redecls_end());
  if (m_config.m_printQualifiers || size > 1) {
    OUT_QATTR_INT("Redeclarable::", "redecls_size()", size);
  }
}


void PrintClangASTNodes::printDecl(clang::Decl const *decl)
{
  #define DECL_FLAG(flagName) \
    (SPY(Decl, decl, flagName)? " " #flagName : "")

  OUT_OBJECT(getDeclIDStr(decl));

  if (m_config.m_printAddresses) {
    OUT_QATTR_STRING("", "address", decl);
  }

  // Don't dig into things like __va_list_tag, which are just noise for
  // my present purposes.
  if (auto namedDecl = dyn_cast<clang::NamedDecl>(decl)) {
    if (namedDecl->isReserved(getLangOptions()) !=
        clang::ReservedIdentifierStatus::NotReserved) {
      OUT_QATTR_STRING("", "skipping reserved",
        doubleQuote(namedDecl->getQualifiedNameAsString()));
      return;
    }
  }

  OUT_QATTR_PTR("Decl::", "NextInContext",
    getDeclIDStr(decl->getNextDeclInContext()));

  OUT_QATTR_STRING("Decl::", "moduleOwnershipKind",
    moduleOwnershipKindStr(decl->getModuleOwnershipKind()));

  // When using shorter names, the semantic DC is more important than
  // the lexical DC, and I want a short name so I can have a short arrow
  // pointing to that DC.
  OUT_QATTR_PTR("Decl::", shortAndLongForms("DC", "semanticDeclContext"),
    getDeclIDStr(declFromDC(decl->getDeclContext())));

  OUT_QATTR_PTR("Decl::", shortAndLongForms("LDC", "lexicalDeclContext"),
    getDeclIDStr(declFromDC(decl->getLexicalDeclContext())));

  OUT_QATTR_STRING("Decl::", "Loc",
    locStr(decl->getLocation()));

  // The qualifier here is unconditional because just "flags" is not
  // specific enough.
  OUT_QATTR_BITSET("", "Decl::flags",
    DECL_FLAG(InvalidDecl) <<
    DECL_FLAG(HasAttrs) <<
    DECL_FLAG(Implicit) <<
    DECL_FLAG(Used) <<
    DECL_FLAG(Referenced) <<
    DECL_FLAG(TopLevelDeclInObjCContainer) <<
    DECL_FLAG(FromASTFile));

  // In the diagrams, I'd like to be able to call attention to implicit
  // nodes by showing this field when present, without always showing
  // the flags field or the rest of its bits.
  if (decl->isImplicit()) {
    OUT_ATTR_STRING("Decl::Implicit", "true");
  }

  OUT_QATTR_STRING("Decl::", "Access",
    accessSpecifierStr(decl->getAccess()));

  OUT_QATTR_STRING("Decl::", "IdentifierNamespace",
    identifierNamespaceStr(
      static_cast<enum clang::Decl::IdentifierNamespace>(
        SPY(Decl, decl, IdentifierNamespace))));

  OUT_QATTR_STRING("Decl::", "Linkage",
    (SPY(Decl, decl, hasCachedLinkage)?
       linkageStr(SPY(Decl, decl, getCachedLinkage))
       : string("(not set)")));

  if (decl->hasAttrs()) {
    unsigned i = 0;
    for (clang::Attr *attr : decl->attrs()) {
      OUT_QATTR_PTR("Decl::", "Attr[" << (i++) << "]",
        getAttrIDStr(attr));
    }
  }

  if (auto declContext = dyn_cast<clang::DeclContext>(decl)) {
    printDeclContext(declContext);
  }

  #undef DECL_FLAG

  // Doing all of the dispatch here is not as efficient as it could be
  // (since we could instead do some of it in subclass print routines),
  // but is easy to organize.
  PRINT_IF_SUBCLASS(decl, NamedDecl)
  PRINT_IF_SUBCLASS(decl, ValueDecl)
  PRINT_IF_SUBCLASS(decl, DeclaratorDecl)
  PRINT_IF_SUBCLASS(decl, VarDecl)
  PRINT_IF_SUBCLASS(decl, ParmVarDecl)
  PRINT_IF_SUBCLASS(decl, FunctionDecl)
  PRINT_IF_SUBCLASS(decl, FieldDecl)
  PRINT_IF_SUBCLASS(decl, TypeDecl)
  PRINT_IF_SUBCLASS(decl, TagDecl)
  PRINT_IF_SUBCLASS(decl, RecordDecl)
  PRINT_IF_SUBCLASS(decl, CXXRecordDecl)
  PRINT_IF_SUBCLASS(decl, CXXMethodDecl)
  PRINT_IF_SUBCLASS(decl, CXXConstructorDecl)
  PRINT_IF_SUBCLASS(decl, FriendDecl)
  PRINT_IF_SUBCLASS(decl, TemplateDecl)
  PRINT_IF_SUBCLASS(decl, RedeclarableTemplateDecl)
  PRINT_IF_SUBCLASS(decl, FunctionTemplateDecl)
  PRINT_IF_SUBCLASS(decl, TemplateTypeParmDecl)
  PRINT_IF_SUBCLASS(decl, ClassTemplateSpecializationDecl)
  PRINT_IF_SUBCLASS(decl, ClassTemplatePartialSpecializationDecl)
  PRINT_IF_SUBCLASS(decl, ClassTemplateDecl)
  PRINT_IF_SUBCLASS(decl, ClassScopeFunctionSpecializationDecl)
}


void PrintClangASTNodes::printNamedDecl(clang::NamedDecl const *decl)
{
  OUT_QATTR_STRING("NamedDecl::", "Name",
    declarationNameStr(decl->getDeclName()));
}


void PrintClangASTNodes::printValueDecl(clang::ValueDecl const *decl)
{
  OUT_QATTR_QUALTYPE("ValueDecl::", "DeclType",
    decl->getType());
}


void PrintClangASTNodes::printDeclaratorDecl(clang::DeclaratorDecl const *decl)
{
  OUT_QATTR_STRING("DeclaratorDecl::", "TInfo",
    typeSourceInfoStr(decl->getTypeSourceInfo()));

  if (SPY(DeclaratorDecl, decl, hasExtInfo)) {
    OUT_QATTR_PTR("DeclaratorDecl::ExtInfo::", "TrailingRequiresClause",
      getStmtIDStr(decl->getTrailingRequiresClause()));

    // TODO: QualifierLoc has more information than this prints, so it
    // would be nice to dig deeper into that structure.
    OUT_QATTR_STRING("DeclaratorDecl::ExtInfo::", "QualifierLoc",
      nestedNameSpecifierLocStr(decl->getQualifierLoc()));

    OUT_QATTR_STRING("DeclaratorDecl::QualifierInfo::", "NumTemplParamLists",
      decl->getNumTemplateParameterLists());

    for (unsigned i=0; i < decl->getNumTemplateParameterLists(); ++i) {
      clang::TemplateParameterList *paramList =
        decl->getTemplateParameterList(i);

      printTemplateParameterList(
        "DeclaratorDecl::QualifierInfo::",
        shortAndLongForms(
          stringb("TPL[" << i << "]"),
          stringb("TemplParamLists[" << i << "]")),
        paramList);
    }
  }
  else {
    OUT_QATTR_STRING("DeclaratorDecl::", "ExtInfo", "absent");
  }

  OUT_QATTR_STRING("DeclaratorDecl::", "InnerLocStart",
    locStr(decl->getInnerLocStart()));
}


void PrintClangASTNodes::printVarDecl(clang::VarDecl const *decl)
{
  #define EVALUATEDSTMT_FLAG(flagName) \
    (evalStmt->flagName? " " #flagName : "")

  llvm::PointerUnion<clang::Stmt *, clang::EvaluatedStmt *> init =
    SPY(VarDecl, decl, Init);

  clang::EvaluatedStmt *evalStmt =
    init.is<clang::EvaluatedStmt*>()? init.get<clang::EvaluatedStmt*>() : nullptr;

  clang::Stmt *initStmt =

    init.is<clang::Stmt*>()?
      init.get<clang::Stmt*>() :

    evalStmt ?
      // If clang >= 17, 'Value' is a 'LazyDeclStmtPtr'.
      IF_CLANG_17(evalStmt->Value.get(getExternalSource()),
                  evalStmt->Value) :

    nullptr;

  OUT_QATTR_PTR("VarDecl::", "InitStmt",
    getStmtIDStr(initStmt));

  if (evalStmt) {
    OUT_QATTR_BITSET("VarDecl::", "EvaluatedStmt flags",
      EVALUATEDSTMT_FLAG(WasEvaluated) <<
      EVALUATEDSTMT_FLAG(IsEvaluating) <<
      EVALUATEDSTMT_FLAG(HasConstantInitialization) <<
      EVALUATEDSTMT_FLAG(HasConstantDestruction) <<
      EVALUATEDSTMT_FLAG(HasICEInit) <<
      EVALUATEDSTMT_FLAG(CheckedForICEInit));

    OUT_QATTR_STRING("VarDecl::EvaluatedStmt::", "Evaluated",
      apValueStr(&(evalStmt->Evaluated)));
  }

  OUT_QATTR_STRING("VarDecl::VarDeclBitfields::", "SClass",
    storageClassStr(decl->getStorageClass()));

  OUT_QATTR_STRING("VarDecl::VarDeclBitfields::", "TSCSpec",
    threadStorageClassSpecifierStr(decl->getTSCSpec()));

  OUT_QATTR_STRING("VarDecl::VarDeclBitfields::", "InitStyle",
    initializationStyleStr(decl->getInitStyle()));

  OUT_QATTR_STRING("VarDecl::VarDeclBitfields::", "ARCPseudoStrong",
    decl->isARCPseudoStrong());

  #undef EVALUATEDSTMT_FLAG
}


void PrintClangASTNodes::printParmVarDecl(clang::ParmVarDecl const *decl)
{
  unsigned parameterIndex = decl->getFunctionScopeIndex();

  string parameterIndexExtra;
  {
    unsigned smallParameterIndex = SPY(ParmVarDecl, decl, ParameterIndex);
    if (smallParameterIndex != parameterIndex) {
      // Print the full value as an annotation.  The large value is
      // stored separately from the node, in the ASTContext using a map.
      parameterIndexExtra = stringb(" (large: " << parameterIndex << ")");

      // But I want to print what's really stored in the node as the
      // main value.
      parameterIndex = smallParameterIndex;
    }
  }

  OUT_QATTR_BITSET("ParmVarDecl::", "ParmVarDeclBits flags",
    (decl->hasInheritedDefaultArg()? " HasInheritedDefaultArg" : "") <<
    (decl->isKNRPromoted()?          " IsKNRPromoted"          : "") <<
    (decl->isObjCMethodParameter()?  " IsObjCMethodParam"      : ""));

  // The comments at the declaration suggest that DAK_None might be
  // used when there is no default argument, but I see DAK_Normal in
  // that case.
  OUT_QATTR_STRING("ParmVarDecl::ParmVarDeclBits::", "DefaultArgKind",
    clang::VarDeclSpy::defaultArgKindStr(
      SPY(ParmVarDecl, decl, DefaultArgKind)));

  // The "scope depth" here refers to something like
  // 'int f(int (*fp)(int x), int y)' having 'x' at a depth of 1.
  OUT_QATTR_STRING("ParmVarDecl::ParmVarDeclBits::", "ScopeDepthOrObjCQuals",
    (decl->isObjCMethodParameter()?
       (unsigned)(decl->getObjCDeclQualifier()) : // I don't care enough to stringify this.
       decl->getFunctionScopeDepth()));

  OUT_QATTR_STRING("ParmVarDecl::ParmVarDeclBits::", "ParameterIndex",
    parameterIndex << parameterIndexExtra);
}


void PrintClangASTNodes::printFunctionDecl(clang::FunctionDecl const *decl)
{
  printRedeclarable(decl);

  OUT_QATTR_STRING("FunctionDecl::FunctionDeclBits::", "SClass",
    storageClassStr(decl->getStorageClass()));

  OUT_QATTR_BITSET("FunctionDecl::", "FunctionDeclBits",
    (decl->isInlined()?
         " IsInline" : "") <<
    (decl->isInlineSpecified()?
         " IsInlineSpecified" : "") <<
    (decl->isVirtualAsWritten()?
         " IsVirtualAsWritten" : "") <<
    (decl->isPure()?
         " IsPure" : "") <<
    (decl->hasInheritedPrototype()?
         " HasInheritedPrototype" : "") <<
    (decl->hasWrittenPrototype()?
         " HasWrittenPrototype" : "") <<
    (SPY(FunctionDecl, decl, isDeletedBit)?
         " IsDeleted" : "") <<
    (decl->isTrivial()?
         " IsTrivial" : "") <<
    (decl->isTrivialForCall()?
         " IsTrivialForCall" : "") <<
    (decl->isDefaulted()?
         " IsDefaulted" : "") <<
    (decl->isExplicitlyDefaulted()?
         " IsExplicitlyDefaulted" : "") <<

    // At first it appears this call is insufficient because the bit
    // could be set but the pointer null.  However, there are only two
    // places in Sema that set the DFI, and both set it to a non-null
    // value, so this should accurately get the bit.
    (decl->getDefaultedFunctionInfo()?
         " HasDefaultedFunctionInfo" : "") <<

    (decl->hasImplicitReturnZero()?
         " HasImplicitReturnZero" : "") <<
    (decl->isLateTemplateParsed()?
         " IsLateTemplateParsed" : "") <<
    (decl->instantiationIsPending()?
         " InstantiationIsPending" : "") <<
    (decl->isConstexpr()?
         stringb(" " << constexprSpecKindStr(decl->getConstexprKind())) :
         string("")) <<
    (decl->usesSEHTry()?
         " UsesSEHTry" : "") <<
    (decl->hasSkippedBody()?
         " HasSkippedBody" : "") <<
    (decl->willHaveBody()?
         " WillHaveBody" : "") <<
    (decl->isMultiVersion()?              // TODO: not accurate
         " IsMultiVersion" : "") <<

    // TODO: This pertains to CXXDeductionGuideDecl.
    //(decl->isCopyDeductionCandidate()?
    //     " IsCopyDeductionCandidate" : "") <<

    (SPY(FunctionDecl, decl, hasODRHash)?
         " HasODRHash" : "") <<
    (decl->UsesFPIntrin()?
         " UsesFPIntrin" : "") <<

    "");

  if (decl->getNumParams() > 0) {
    for (unsigned i=0; i < decl->getNumParams(); ++i) {
      // The name of the private data member is 'ParamInfo', but it can
      // be unambiguously shortened to 'Param'.
      OUT_QATTR_PTR("FunctionDecl::",
        "Param[" << i << "]",
          getDeclIDStr(decl->getParamDecl(i)));
    }
  }
  else {
    OUT_QATTR_STRING("FunctionDecl::", "ParamInfo", "(empty)");
  }

  if (clang::FunctionDecl::DefaultedFunctionInfo *dfi =
        decl->getDefaultedFunctionInfo()) {
    // The DFI object is shared between a template and an instantiation
    // when the lookup set is the same, so here I am potentially
    // printing something multiple times that is physically only stored
    // once.  For now I'll just print the address, if that is not
    // disabled, to have some ability to recognize the sharing.  A
    // possible further improvement would be to number these and print
    // them separately.
    if (m_config.m_printAddresses) {
      OUT_QATTR_STRING("FunctionDecl::", "DefaultedFunctionInfo", dfi);
    }

    int i=0;
    for (clang::DeclAccessPair const &apair : dfi->getUnqualifiedLookups()) {
      OUT_QATTR_PTR("FunctionDecl::", "DefaultedFunctionInfo[" << i << "].NamedDecl",
        getDeclIDStr(apair.getDecl()));

      OUT_QATTR_STRING("FunctionDecl::", "DefaultedFunctionInfo[" << i << "].Access",
        accessSpecifierStr(apair.getAccess()));

      ++i;
    }

    if (i == 0) {
      OUT_QATTR_STRING("FunctionDecl::", "DefaultedFunctionInfo", "(empty)");
    }
  }

  else {
    // I do not use 'getBody' for this because I want to know about this
    // particular node, not some potentially different redeclaration
    // that has a body.
    OUT_QATTR_PTR("FunctionDecl::", "Body",
      getStmtIDStr(SPY(FunctionDecl, decl, Body).
                     get(getExternalSource())));
  }

  OUT_QATTR_STRING("FunctionDecl::", "ODRHash",
    (SPY(FunctionDecl, decl, ODRHash)));

  OUT_QATTR_STRING("FunctionDecl::", "EndRangeLoc",
    locStr(SPY(FunctionDecl, decl, EndRangeLoc)));

  OUT_QATTR_STRING("FunctionDecl::", "DefaultKWLoc",
    locStr(SPY(FunctionDecl, decl, DefaultKWLoc)));

  // This is computed from 'TemplateOrSpecialization', but I think the
  // computation is questionable so I'm printing its result.
  clang::FunctionDecl::TemplatedKind templatedKind =
    decl->getTemplatedKind();
  OUT_QATTR_STRING("FunctionDecl::", "getTemplatedKind()",
    templatedKindStr(templatedKind));

  char const * const qualifier = "FunctionDecl::";
  char const * const label = "TemplateOrSpecialization";

  // The 'TemplateOrSpecialization' discriminated union can be
  // indirectly discriminated via 'getTemplatedKind()'.
  switch (templatedKind) {
    case clang::FunctionDecl::TK_NonTemplate:
      OUT_QATTR_STRING(qualifier, label, "null");
      break;

    case clang::FunctionDecl::TK_FunctionTemplate:
      OUT_QATTR_PTR(qualifier << label,
        shortAndLongForms("FTD", ".NamedDecl (described)"),
          getDeclIDStr(decl->getDescribedFunctionTemplate()));
      break;

    // This case means this an ordinary function that was declared
    // inside the body of a function template.
    //
    // TODO: Really?  Naively there seem to be many cases this can't
    // account for, like being inside a member of a class template, or a
    // function template inside a function template.  And is this only
    // for GNU nested functions, or is that a more general C++ feature?
    case clang::FunctionDecl::TK_DependentNonTemplate:
      OUT_QATTR_PTR(qualifier, label << ".NamedDecl (inst from within)",
        getDeclIDStr(decl->getInstantiatedFromDecl()));
      break;

    case clang::FunctionDecl::TK_MemberSpecialization: {
      clang::MemberSpecializationInfo *memberSpecInfo =
        decl->getMemberSpecializationInfo();
      assert(memberSpecInfo);

      OUT_QATTR_PTR(qualifier << label << ".",
        shortAndLongForms("MSI", "MemberSpecializationInfo"),
          getOrCreateMemberSpecializationInfoIDStr(memberSpecInfo));
      break;
    }

    case clang::FunctionDecl::TK_FunctionTemplateSpecialization: {
      clang::FunctionTemplateSpecializationInfo *funcSpecInfo =
        decl->getTemplateSpecializationInfo();
      assert(funcSpecInfo);

      OUT_QATTR_PTR(qualifier << label << ".",
        shortAndLongForms("FTSI", "FunctionTemplateSpecializationInfo"),
          getOrCreateFunctionTemplateSpecializationInfoIDStr(funcSpecInfo));
      break;
    }

    case clang::FunctionDecl::TK_DependentFunctionTemplateSpecialization: {
      clang::DependentFunctionTemplateSpecializationInfo *depFuncSpecInfo =
        decl->getDependentSpecializationInfo();
      assert(depFuncSpecInfo);

      OUT_QATTR_PTR(qualifier << label << ".",
        shortAndLongForms("DFTSI", "DependentFunctionTemplateSpecializationInfo"),
          getOrCreateDependentFunctionTemplateSpecializationInfoIDStr(
            depFuncSpecInfo));
      break;
    }

    // I'm omitting a 'default' in hopes the compiler will warn if a
    // case is missing here.
  }

  printDeclarationNameInfoLocInfo("FunctionDecl::", "DNLoc",
    decl->getNameInfo());

  if (SPY(FunctionDecl, decl, isDeletedBit) &&
      !isa<clang::CXXMethodDecl>(decl)) {
    // Due to an apparent bug, the end location is wrong.  Compute a
    // corrected location.
    clang::SourceLocation fixedEndLoc = getEndOfEqDelete(decl);
    OUT_QATTR_STRING("FunctionDecl::", "fixedEndLoc (computed)",
      locStr(fixedEndLoc));
  }

  // Print the result of various queries so I can try to correlate them
  // with the underlying data.

  OUT_QATTR_PTR("FunctionDecl::", "getInstantiatedFromMemberFunction()",
    getDeclIDStr(decl->getInstantiatedFromMemberFunction()));

  // 'getTemplatedKind()' was printed above.

  OUT_QATTR_PTR("FunctionDecl::", "getMemberSpecializationInfo()",
    getMemberSpecializationInfoIDStr(
      decl->getMemberSpecializationInfo()));

  OUT_QATTR_PTR("FunctionDecl::", "getDescribedFunctionTemplate()",
    getDeclIDStr(decl->getDescribedFunctionTemplate()));

  OUT_QATTR_PTR("FunctionDecl::", "getTemplateSpecializationInfo()",
    getFunctionTemplateSpecializationInfoIDStr(
      decl->getTemplateSpecializationInfo()));

  OUT_QATTR_PTR("FunctionDecl::", "getTemplateInstantiationPattern(true)",
    getDeclIDStr(decl->getTemplateInstantiationPattern(true)));

  OUT_QATTR_PTR("FunctionDecl::", "getTemplateInstantiationPattern(false)",
    getDeclIDStr(decl->getTemplateInstantiationPattern(false)));

  OUT_QATTR_PTR("FunctionDecl::", "getPrimaryTemplate()",
    getDeclIDStr(decl->getPrimaryTemplate()));

  // TODO: Call getDependentSpecializationInfo().

  // For the moment, only print this in verbose mode so I don't have
  // complaints about my diagrams' graphs being out of date...
  if (m_config.m_printQualifiers) {
    OUT_QATTR_STRING("FunctionDecl::", "getExceptionSpecType()",
      exceptionSpecificationTypeStr(decl->getExceptionSpecType()));
  }
}


void PrintClangASTNodes::printFieldDecl(clang::FieldDecl const *decl)
{
  // DeclaratorDecl is automatically printed by 'VisitDecl'.

  // Mergeable does not have any fields.

  OUT_QATTR_BITSET("", "FieldDecl flags",

    (decl->isBitField()?
           " BitField" : "") <<

    (decl->isMutable()?
           " Mutable" : ""));

  // I don't know what this is.
  OUT_QATTR_STRING("FieldDecl::", "CachedFieldIndex",
    SPY(FieldDecl, decl, CachedFieldIndex));

  // This is not *precisly* what is stored, but the exact
  // representation was changed in Clang 17, and using the public API
  // avoids a little breakage.
  OUT_QATTR_STRING("FieldDecl::", "InitStorageKind",
    inClassInitStyleStr(decl->getInClassInitStyle()));

  // The next three fields are encoded with one pointer in the FieldDecl
  // itself, occasionally using an auxiliary structure, with the cases
  // discriminated by 'BitField' and 'InitStorageKind', in a moderately
  // complicated way.  I'll just print them as three optional fields.

  if (decl->isBitField()) {
    OUT_QATTR_PTR("FieldDecl::", "BitWidth",
      getStmtIDStr(decl->getBitWidth()));
  }

  if (decl->hasInClassInitializer()) {
    // The initializer might be null because of delayed parsing despite
    // the flag saying one should be present.
    //
    // Also the expression might not already be numbered due to:
    // https://github.com/llvm/llvm-project/issues/64916
    OUT_QATTR_PTR("FieldDecl::", "Init",
      getOrCreateStmtIDStr(decl->getInClassInitializer()));
  }

  if (decl->hasCapturedVLAType()) {
    OUT_QATTR_STRING("FieldDecl::", "CapturedVLAType",
      typeAndKindStr(decl->getCapturedVLAType()));
  }
}


void PrintClangASTNodes::printTypeDecl(clang::TypeDecl const *decl)
{
  clang::Type const *typeForDecl = decl->getTypeForDecl();

  OUT_QATTR_TYPE("TypeDecl::", "TypeForDecl",
    typeForDecl);
  OUT_QATTR_STRING("TypeDecl::", "LocStart",
    locStr(decl->getBeginLoc()));
}


void PrintClangASTNodes::printTagDecl(clang::TagDecl const *decl)
{
  // Note: The DeclContext is printed by 'VisitDecl'.

  printRedeclarable(decl);

  OUT_QATTR_STRING("TagDecl::", "TagDeclBits",

    tagTypeKindStr(decl->getTagKind()) <<

    (decl->isCompleteDefinition()?
         " IsCompleteDefinition" : "") <<

    (decl->isBeingDefined()?
         " IsBeingDefined" : "") <<

    (decl->isEmbeddedInDeclarator()?
         " IsEmbeddedInDeclarator" : "") <<

    (decl->isFreeStanding()?
         " IsFreeStanding" : "") <<

    (decl->mayHaveOutOfDateDef()?
         " MayHaveOutOfDateDef" : "") <<

    (decl->isCompleteDefinitionRequired()?
         " IsCompleteDefinitionRequired" : "") <<

    (decl->isThisDeclarationADemotedDefinition()?
         " IsThisDeclarationADemotedDefinition" : "") <<

    "");

  // In the diagrams, I'd like this called out separately.
  OUT_QATTR_STRING("TagDecl::TagDecl", "Bits.IsCompleteDefinition",
    (decl->isCompleteDefinition()? "true" : "false"));

  OUT_QATTR_STRING("TagDecl::", "BraceRange",
    sourceRangeStr(decl->getBraceRange()));

  // Note: The documentation talks about a 'VarDecl' case that seems to
  // not exist.
  llvm::PointerUnion<clang::TypedefNameDecl *,
                     clang::QualifierInfo *> tndoq =
    SPY(TagDecl, decl, TypedefNameDeclOrQualifier);

  if (tndoq.is<clang::TypedefNameDecl*>()) {
    OUT_QATTR_PTR("TagDecl::", "TypedefNameDeclOrQualifier.name",
      getDeclIDStr(tndoq.get<clang::TypedefNameDecl*>()));
  }
  else {
    if (clang::QualifierInfo *qi = tndoq.get<clang::QualifierInfo*>()) {
      char const * const qualifier =
        "TagDecl::TypedefNameDeclOrQualifier.";
      string label = shortAndLongForms("q->", "qualifier->");

      OUT_QATTR_STRING(qualifier, label << "QualifierLoc",
        nestedNameSpecifierLocStr(qi->QualifierLoc));

      OUT_QATTR_STRING(qualifier, label << "NumTemplParamLists",
        qi->NumTemplParamLists);

      for (unsigned i=0; i < qi->NumTemplParamLists; ++i) {
        OUT_QATTR_STRING(qualifier, label << "TemplParamLists[" << i << "]",
          doubleQuote(templateParameterListStr(qi->TemplParamLists[i])));
      }
    }
    else {
      // I think this would violate the data structure invariants, but
      // I'm being defensive.
      OUT_QATTR_STRING("TagDecl::", "TypedefNameDeclOrQualifier.qualifier", "null");
    }
  }
}


void PrintClangASTNodes::printRecordDecl(clang::RecordDecl const *decl)
{
  OUT_QATTR_BITSET("RecordDecl::", "RecordDeclBits",

    (decl->hasFlexibleArrayMember()?
         " HasFlexibleArrayMember" : "") <<

    (decl->isAnonymousStructOrUnion()?
           " AnonymousStructOrUnion" : "") <<

    (decl->hasObjectMember()?
         " HasObjectMember" : "") <<

    (decl->hasVolatileMember()?
         " HasVolatileMember" : "") <<

    (decl->hasLoadedFieldsFromExternalStorage()?
            " LoadedFieldsFromExternalStorage" : "") <<

    (decl->isNonTrivialToPrimitiveDefaultInitialize()?
           " NonTrivialToPrimitiveDefaultInitialize" : "") <<

    (decl->isNonTrivialToPrimitiveCopy()?
           " NonTrivialToPrimitiveCopy" : "") <<

    (decl->isNonTrivialToPrimitiveDestroy()?
           " NonTrivialToPrimitiveDestroy" : "") <<

    (decl->hasNonTrivialToPrimitiveDefaultInitializeCUnion()?
         " HasNonTrivialToPrimitiveDefaultInitializeCUnion" : "") <<

    (decl->hasNonTrivialToPrimitiveDestructCUnion()?
         " HasNonTrivialToPrimitiveDestructCUnion" : "") <<

    (decl->hasNonTrivialToPrimitiveCopyCUnion()?
         " HasNonTrivialToPrimitiveCopyCUnion" : "") <<

    (decl->isParamDestroyedInCallee()?
         " ParamDestroyedInCallee" : "") <<

    " " << argPassingKindStr(decl->getArgPassingRestrictions()) <<

    (decl->isRandomized()?
         " IsRandomized" : "") <<

    "");

  // This call causes the ODRHash to be computed, which I don't want,
  // but the API leaves me no choice, and none of my access control
  // circumvention tricks would work here.
  //
  // Computing the class ODRHash also computes the hashes of the
  // methods.  I really dislike that.  But maybe it's not so bad
  // seeing what some of these hashes behave like.
  //
  // Also note a BUG in the design: this value is used as a boolean
  // flag, but nothing prevents a hash value of 0 from arising, in
  // which case every attempt to get the hash would cause it to be
  // recomputed.  (Not a big deal, but would be easy to fix so long
  // as there isn't some mysterious correspondence with another
  // shift-by-6 computation that has to be preserved.)
  OUT_QATTR_STRING("RecordDecl::", "ODRHash",
    const_cast<clang::RecordDecl*>(decl)->getODRHash());

  // There are no other fields of RecordDecl.  The members are recorded
  // in the linked list maintained by DeclContext.
}


void PrintClangASTNodes::printCXXRecordDecl(clang::CXXRecordDecl const *decl)
{
  char const * const qualifier = "CXXRecordDecl::";

  clang::CXXRecordDeclSpy::CXXRecordDecl_DefinitionData *defData =
    SPY(CXXRecordDecl, decl, DefinitionData);

  // The DDs are never numbered in advance.  (I'm planning to remove the
  // advance numbering thing altogether at some point.)
  OUT_QATTR_PTR(qualifier, "DefinitionData",
    getOrCreateFake_CXXRecordDecl_DefinitionDataIDStr(toFakeDD(defData)));

  llvm::PointerUnion<clang::ClassTemplateDecl *,
                     clang::MemberSpecializationInfo *>
    templateOrInstantiation =
      SPY(CXXRecordDecl, decl, TemplateOrInstantiation);

  if (templateOrInstantiation.isNull()) {
    OUT_QATTR_STRING(qualifier, "TemplateOrInstantiation", "null");
  }
  else if (auto classTemplateDecl =
             templateOrInstantiation.dyn_cast<clang::ClassTemplateDecl*>()) {
    OUT_QATTR_PTR(qualifier,
      shortAndLongForms("CTD", "TemplateOrInstantiation.ctd"),
        getDeclIDStr(classTemplateDecl));
  }
  else if (auto memberSpecializationInfo =
             templateOrInstantiation.dyn_cast<clang::MemberSpecializationInfo*>()) {
    OUT_QATTR_PTR(qualifier,
      shortAndLongForms("MSI", "TemplateOrInstantiation.msi"),
        getOrCreateMemberSpecializationInfoIDStr(memberSpecializationInfo));
  }
  else {
    OUT_QATTR_STRING(qualifier, "TemplateOrInstantiation", "unknown type?");
  }

  /*
    Check proposed invariant:

      forall CXXRecordDecl *decl:
        forall TagDecl *redecl in decl->redecls():
          cast<CXXRecordDecl>(redecl)->DefinitionData ==
          decl->DefinitionData

    That is, all of the redeclarations have the same DefinitionData.
  */
  for (clang::TagDecl const *redecl : decl->redecls()) {
    if (auto crd = dyn_cast<clang::CXXRecordDecl>(redecl)) {
      PRINT_ASSERT(SPY(CXXRecordDecl, crd, DefinitionData) == defData);
    }
    else {
      PRINT_ASSERT_FAIL("Redecl was not a CXXRecordDecl");
    }
  }

  // Print results of queries.  For now these just focus on template-
  // related things.

// Print the result of 'decl->query', which yields a Decl pointer.
#define OUT_QATTR_QUERY_DECL_PTR(qualifier, decl, query) \
  OUT_QATTR_PTR(qualifier, #query,                       \
    getDeclIDStr(decl->query));

  OUT_QATTR_QUERY_DECL_PTR(qualifier,
    decl, getDependentLambdaCallOperator());

  OUT_QATTR_QUERY_DECL_PTR(qualifier,
    decl, getInstantiatedFromMemberClass());

  OUT_QATTR_QUERY_DECL_PTR(qualifier,
    decl, getDescribedClassTemplate());

  OUT_QATTR_QUERY_DECL_PTR(qualifier,
    decl, getTemplateInstantiationPattern());

  // More ad-hoc queries.

  OUT_QATTR_QUERY_DECL_PTR(qualifier,
    decl, getDestructor());

  if (decl->isThisDeclarationADefinition()) {
    OUT_QATTR_BOOL(qualifier, "needsImplicitDestructor()",
      decl->needsImplicitDestructor());
  }
}


void PrintClangASTNodes::printCXXMethodDecl(clang::CXXMethodDecl const *decl)
{
  // There's no new data in CXXMethodDecl.  Even its bits are already
  // printed as part of FunctionDecl.
}


void PrintClangASTNodes::printCXXConstructorDecl(
  clang::CXXConstructorDecl const *decl)
{
  // Not doing this yet.
  //clang::ExplicitSpecifier explicitSpecifier =
  //  SPY(CXXConstructorDecl, decl, getExplicitSpecifierInternal);

  char const *qualifier = "CXXConstructorDecl::";

  OUT_QATTR_STRING(qualifier, "CXXConstructorDeclBits.NumCtorInitializers",
    decl->getNumCtorInitializers());

  OUT_QATTR_BITSET(qualifier, "CXXConstructorDeclBits flags",
    (decl->isInheritingConstructor()?
         " IsInheritingConstructor" : "") <<
    (SPY(CXXConstructorDecl, decl, HasTrailingExplicitSpecifier)?
                                 " HasTrailingExplicitSpecifier" : ""));
    // Can't get 'IsSimpleExplicit'.

  // Unfortunately, querying the real data is challenging here.
  OUT_QATTR_STRING(qualifier, "getExplicitSpecifierInternal()", "TODO");

  OUT_QATTR_STRING(qualifier, "InheritedConstructor", "TODO");

  unsigned i = 0;
  for (clang::CXXCtorInitializer const *init : decl->inits()) {
    printCXXCtorInitializer(qualifier,
      stringb("Init[" << (i++) << "]"),
        init);
  }
}


void PrintClangASTNodes::printFriendDecl(clang::FriendDecl const *decl)
{
  char const *qualifier = "FriendDecl::";

  if (clang::TypeSourceInfo const *tsi = decl->getFriendType()) {
    OUT_QATTR_STRING(qualifier, "Friend.TSI",
      typeSourceInfoStr(decl->getFriendType()));
  }
  else {
    OUT_QATTR_DECL(qualifier, "Friend.ND",
      decl->getFriendDecl());
  }

  OUT_QATTR_DECL(qualifier, "NextFriend",
    SPY(FriendDecl, decl, getNextFriend));

  OUT_QATTR_LOC(qualifier, "FriendLoc",
    decl->getFriendLoc());

  OUT_QATTR_BOOL(qualifier, "UnsupportedFriend",
    decl->isUnsupportedFriend());

  OUT_QATTR_INT(qualifier, "NumTPLists",
    decl->getFriendTypeNumTemplateParameterLists());

  for (unsigned i=0; i < decl->getFriendTypeNumTemplateParameterLists(); ++i) {
    printTemplateParameterList(qualifier,
      shortAndLongForms("TP", "TemplateParams"),
        decl->getFriendTypeTemplateParameterList(i));
  }
}


void PrintClangASTNodes::printTemplateDecl(clang::TemplateDecl const *decl)
{
  OUT_QATTR_PTR("TemplateDecl::", shortAndLongForms("TdD", "TemplatedDecl"),
    getDeclIDStr(decl->getTemplatedDecl()));

  printTemplateParameterList("TemplateDecl::",
    shortAndLongForms("TP", "TemplateParams"),
      decl->getTemplateParameters());
}


void PrintClangASTNodes::printRedeclarableTemplateDecl(
  clang::RedeclarableTemplateDecl const *decl)
{
  printRedeclarable(decl);

  OUT_QATTR_PTR("RedeclarableTemplateDecl::",
    "getInstantiatedFromMemberTemplate()",
      getDeclIDStr(decl->getInstantiatedFromMemberTemplate()));
}


void PrintClangASTNodes::printFunctionTemplateDecl(
  clang::FunctionTemplateDecl const *functionTemplateDecl)
{
  clang::FunctionTemplateDecl_Common *common =
    SPY(FunctionTemplateDecl, functionTemplateDecl, getCommonPtr);

  m_mapCommonToFunctionTemplateDecl.insert(std::make_pair(
    common, functionTemplateDecl));

  OUT_QATTR_PTR("FunctionTemplateDecl::",
    shortAndLongForms("Cmn", "Common"),
      m_numbering.getOrCreateFunctionTemplateDecl_CommonIDStr(
        common));
}


void PrintClangASTNodes::printTemplateTypeParmDecl(
  clang::TemplateTypeParmDecl const *decl)
{
  #define TEMPLATETYPEPARMDECL_FLAG(flagName) \
    (SPY(TemplateTypeParmDecl, decl, flagName)? " " #flagName : "")

  OUT_QATTR_BITSET("", "TemplateTypeParmDecl flags",
    TEMPLATETYPEPARMDECL_FLAG(Typename) <<
    TEMPLATETYPEPARMDECL_FLAG(HasTypeConstraint) <<
    TEMPLATETYPEPARMDECL_FLAG(TypeConstraintInitialized) <<
    TEMPLATETYPEPARMDECL_FLAG(ExpandedParameterPack));

  OUT_QATTR_STRING("TemplateTypeParmDecl::", "NumExpanded",
    SPY(TemplateTypeParmDecl, decl, NumExpanded));
}


void PrintClangASTNodes::printClassTemplateSpecializationDecl(
  clang::ClassTemplateSpecializationDecl const *decl)
{
  typedef clang::ClassTemplateSpecializationDeclSpy::
           ClassTemplateSpecializationDecl_SpecializedPartialSpecialization
             SPS;

  llvm::PointerUnion<clang::ClassTemplateDecl *, SPS *>
    specializedTemplate =
      SPY(ClassTemplateSpecializationDecl, decl, SpecializedTemplate);

  char const *qualifier = "ClassTemplateSpecializationDecl::";
  string label = shortAndLongForms("ST", "SpecializedTemplate");

  if (auto ctd = specializedTemplate.dyn_cast<clang::ClassTemplateDecl*>()) {
    OUT_QATTR_PTR(qualifier,
      label << shortAndLongForms(".CTD", ".ClassTemplateDecl"),
        getDeclIDStr(ctd));
  }
  else if (auto sps = specializedTemplate.dyn_cast<SPS*>()) {
    string spsText = shortAndLongForms(".SPS", ".SpecializedPartialSpecialization");
    OUT_QATTR_PTR(qualifier, label << spsText << "->PartialSpecialization",
      getDeclIDStr(sps->PartialSpecialization));
    OUT_QATTR_STRING(qualifier, label << spsText << "->TemplateArgs",
      templateArgumentListOptStr(sps->TemplateArgs));
  }
  else {
    OUT_QATTR_STRING(qualifier, label, "null");
  }


  typedef clang::ClassTemplateSpecializationDeclSpy::
           ClassTemplateSpecializationDecl_ExplicitSpecializationInfo
             ESI;

  qualifier = "ClassTemplateSpecializationDecl::";
  label = shortAndLongForms("EI", "ExplicitInfo");

  ESI *esi = SPY(ClassTemplateSpecializationDecl, decl, ExplicitInfo);
  if (esi) {
    OUT_QATTR_STRING(qualifier, label << "->TypeAsWritten",
      typeSourceInfoStr(esi->TypeAsWritten));
    OUT_QATTR_STRING(qualifier, label << "->ExternLoc",
      locStr(esi->ExternLoc));
    OUT_QATTR_STRING(qualifier, label << "->TemplateKeywordLoc",
      locStr(esi->TemplateKeywordLoc));

    // Inspect 'TypeAsWritten' more closely.
    clang::QualType qtaw = esi->TypeAsWritten->getType();
    if (PRINT_ASSERT(!qtaw.isNull())) {
      // Usually this is a TemplateSpecializationType, but there are
      // also dependent variants, and it might be possible to use a
      // typedef for this too.
      clang::Type const *taw = qtaw.getTypePtr();

      // Print it as a Type.
      OUT_QATTR_PTR(qualifier, label << "->TAW->Ty",
        getTypeIDStr(taw));
    }
  }
  else {
    // If this is null, spell it out rather than using the abbreviation.
    OUT_QATTR_STRING(qualifier, "ExplicitInfo", "null");
  }

  OUT_QATTR_STRING(qualifier, "TemplateArgs",
    doubleQuote(templateArgumentListStr(decl->getTemplateArgs())));
  OUT_QATTR_STRING(qualifier, "PointOfInstantiation",
    locStr(decl->getPointOfInstantiation()));
  OUT_QATTR_STRING(qualifier, "SpecializationKind",
    templateSpecializationKindStr(decl->getSpecializationKind()));

  printTemplateArgumentList(
    qualifier,
    "TemplateArgs",
    decl->getTemplateArgs());

  // Print some query results for ease of correlation.

  OUT_QATTR_PTR(qualifier, "getSpecializedTemplate()",
    getDeclIDStr(decl->getSpecializedTemplate()));



// Print a value that is an llvm::PointerUnion<Type1*, Type2*>, where
// 'Type1' and 'Type2' are both 'Decl' subtypes.
//
// 'abbrev1' and 'abbrev2' are abbreviations for 'Type1' and 'Type2'.
#define OUT_QATTR_DECL_PTR_UNION(qualifier, label, value,        \
                                 Type1, abbrev1, Type2, abbrev2) \
  if ((value).isNull()) {                                        \
    OUT_QATTR_NULL(qualifier, label);                            \
  }                                                              \
  else if ((value).is<Type1*>()) {                               \
    OUT_QATTR_PTR(qualifier, label "." abbrev1,                  \
      getDeclIDStr((value).get<Type1*>()));                      \
  }                                                              \
  else if ((value).is<Type2*>()) {                               \
    OUT_QATTR_PTR(qualifier, label "." abbrev2,                  \
      getDeclIDStr((value).get<Type2*>()));                      \
  }                                                              \
  else {                                                         \
    OUT_QATTR_STRING(qualifier, label,                           \
      "unknown kind!");                                          \
  }

  llvm::PointerUnion<clang::ClassTemplateDecl *,
                     clang::ClassTemplatePartialSpecializationDecl *>
    instantiatedFrom =
      decl->getInstantiatedFrom();
  OUT_QATTR_DECL_PTR_UNION(qualifier, "getInstantiatedFrom()",
    instantiatedFrom,
    clang::ClassTemplateDecl, "CTD",
    clang::ClassTemplatePartialSpecializationDecl, "CTPSD");

  llvm::PointerUnion<clang::ClassTemplateDecl *,
                     clang::ClassTemplatePartialSpecializationDecl *>
    specializedTemplateOrPartial =
      decl->getSpecializedTemplateOrPartial();
  OUT_QATTR_DECL_PTR_UNION(qualifier, "getSpecializedOrPartial()",
    specializedTemplateOrPartial,
    clang::ClassTemplateDecl, "CTD",
    clang::ClassTemplatePartialSpecializationDecl, "CTPSD");
}


void PrintClangASTNodes::printClassTemplatePartialSpecializationDecl(
  clang::ClassTemplatePartialSpecializationDecl const *decl)
{
  char const *qualifier = "ClassTemplatePartialSpecializationDecl::";

  PRINT_ASSERT(decl->getTemplateParameters() != nullptr);
  printTemplateParameterList(qualifier,
    shortAndLongForms("TP", "TemplateParams"),
      decl->getTemplateParameters());

  OUT_QATTR_TODO(qualifier, "ArgsAsWritten");

  llvm::PointerIntPair<clang::ClassTemplatePartialSpecializationDecl *, 1, bool>
    instantiatedFromMember =
      SPY(ClassTemplatePartialSpecializationDecl, decl, InstantiatedFromMember);

  /*
    As elsewhere, this bit is confusing.

    The documentation on the bit says:

      The boolean value will be true to indicate that this class template
      partial specialization was specialized at this level.

    but it is returned by method 'isMemberSpecialization()', which says:

      Determines whether this class template partial specialization
      template was a specialization of a member partial specialization.
  */
  OUT_QATTR_INT(qualifier << "InstantiatedFromMember.", "specdThisLevel",
    instantiatedFromMember.getInt());

  OUT_QATTR_PTR(qualifier, "InstantiatedFromMember" << ifLongForm(".ptr"),
    getDeclIDStr(instantiatedFromMember.getPointer()));
}


void PrintClangASTNodes::printClassTemplateDecl(
  clang::ClassTemplateDecl const *decl)
{
  clang::ClassTemplateDecl_Common *common =
    SPY(ClassTemplateDecl, decl, getCommonPtr);

  m_mapCommonToClassTemplateDecl.insert(std::make_pair(common, decl));

  OUT_QATTR_PTR("ClassTemplateDecl::",
    shortAndLongForms("Cmn", "Common"),
      m_numbering.getOrCreateClassTemplateDecl_CommonIDStr(
        common));

  /*
    Check proposed invariant:

      forall ClassTemplateDecl *decl:
        decl->Common->InjectedClassNameType->
          getAs<TemplateSpecializationType>()->
            Template.getAsTemplateDecl()           ==
        decl->getCanonicalDecl()
  */
  if (auto tst = common->InjectedClassNameType->
                   getAs<clang::TemplateSpecializationType>()) {
    clang::TemplateDecl const *td = tst->getTemplateName().getAsTemplateDecl();
    clang::ClassTemplateDecl const *canonicalTemplateDecl = decl->getCanonicalDecl();
    if (td == canonicalTemplateDecl) {
      ++m_passedAssertions;
    }
    else {
      PRINT_ASSERT_FAILED_NOPREFIX(
        "\n  C->ICNT->TD == canonicalTemplateDecl invariant failed:"
        "\n    C->ICNT->TD: " << getDeclIDStr(td) <<
        "\n    canonicalTemplateDecl: " << getDeclIDStr(canonicalTemplateDecl));
    }
  }
  else {
    PRINT_ASSERT_FAILED_NOPREFIX(
      "\n  C->ICNT->TD == canonicalTemplateDecl invariant failed: "
      "C->ICNT is not a TST");
  }

  /*
    Check proposed invariant:

      forall ClassTemplateDecl *decl:
        decl->TemplatedDecl->TypeForDecl->
          getAs<InjectedClassNameType>()->
            InjectedType                           ==
        decl->Common->InjectedClassNameType
  */
  if (auto icnt = decl->getTemplatedDecl()->getTypeForDecl()->
                    getAs<clang::InjectedClassNameType>()) {
    if (icnt->getInjectedSpecializationType() ==
        common->InjectedClassNameType) {
      ++m_passedAssertions;
    }
    else {
      PRINT_ASSERT_FAILED_NOPREFIX(
        "\n  TD->TFD->IT == C->ICNT invariant failed: "
        "TD->TFD->IT: " <<
        qualTypeIDSyntaxJson(icnt->getInjectedSpecializationType()));
    }

    // This is not the same as 'icnt->getDecl()', which searches among
    // the redeclarations for an "interesting" declaration (!).
    clang::CXXRecordDecl const *icntDecl =
      SPY(InjectedClassNameType, icnt, Decl);

    /*
      Check proposed invariant:

        forall ClassTemplateDecl const *decl:
          decl->TemplatedDecl->TypeForDecl->
            getAs<InjectedClassNameType>()->
              Decl                                   ==
          decl->TemplatedDecl->getCanonicalDecl()
    */
    clang::CXXRecordDecl const *canonicalRecordDecl =
      decl->getTemplatedDecl()->getCanonicalDecl();
    if (icntDecl == canonicalRecordDecl) {
      ++m_passedAssertions;
    }
    else {
      PRINT_ASSERT_FAILED_NOPREFIX(
        "\n  TD->TFD->D == canonicalRecordDecl invariant failed:"
        "\n    TD->TFD->D: " << getDeclIDStr(icntDecl) <<
        "\n    canonicalRecordDecl: " << getDeclIDStr(canonicalRecordDecl));
    }

    /*
      Check proposed invariant:

        forall ClassTemplateDecl const *decl:
          decl->TemplatedDecl->TypeForDecl->
            getAs<InjectedClassNameType>()->
              Decl                                   ==
          decl->TemplatedDecl->TypeForDecl->
            getAs<InjectedClassNameType>()->
              InjectedType->
                getAs<TemplateSpecializationType>()->
                  Template.getAsTemplateDecl()->
                    TemplatedDecl

      That is, within an InjectedClassNameType, its Decl field can be
      computed from its InjectedType field.
    */
    if (auto tst =
          icnt->
            getInjectedSpecializationType()->
              getAs<clang::TemplateSpecializationType>()) {
      if (auto tdecl =
            tst->getTemplateName().getAsTemplateDecl()) {
        if (icntDecl == tdecl->getTemplatedDecl()) {
          ++m_passedAssertions;
        }
        else {
          PRINT_ASSERT_FAILED_NOPREFIX(
            "\n  TD->TFD->D == TD->TFD->IT->T.TD->TD invariant failed: "
            "TD->TFD->D=" << getDeclIDStr(icntDecl) <<
            " TD->TFD->IT->T.TD->TD=" <<
            getDeclIDStr(tdecl->getTemplatedDecl()));
        }
      }
      else {
        PRINT_ASSERT_FAILED_NOPREFIX(
          "\n  TD->TFD->D == TD->TFD->IT->T.TD->TD invariant failed: "
          "TD->TFD->IT->T is not a Template kind");
      }
    }
    else {
      PRINT_ASSERT_FAILED_NOPREFIX(
        "\n  TD->TFD->D == TD->TFD->IT->T.TD->TD invariant failed: "
        "TD->TFD->IT is not a TST");
    }
  }
  else {
    PRINT_ASSERT_FAILED_NOPREFIX(
      "\n  TD->TFD->IT==C->ICNT invariant failed: "
      "TD->TFD is not an ICNT");
  }
}


void PrintClangASTNodes::printClassScopeFunctionSpecializationDecl(
  clang::ClassScopeFunctionSpecializationDecl const *decl)
{
  char const *qualifier = "ClassScopeFunctionSpecializationDecl::";

  OUT_QATTR_PTR(qualifier, "Specialization",
    getDeclIDStr(decl->getSpecialization()));

  printASTTemplateArgumentListInfo(qualifier, "TemplateArgs",
    decl->getTemplateArgsAsWritten());
}


// ------------------ PrintClangASTNodes: statements -------------------
void PrintClangASTNodes::printStmt(clang::Stmt const *stmt)
{
  OUT_OBJECT(getStmtIDStr(stmt));

  if (m_config.m_printAddresses) {
    OUT_QATTR_STRING("", "address", stmt);
  }

  // Dispatch to subclass print routines.
  PRINT_IF_SUBCLASS(stmt, DeclStmt)
  PRINT_IF_SUBCLASS(stmt, CompoundStmt)
  PRINT_IF_SUBCLASS(stmt, ValueStmt)
  PRINT_IF_SUBCLASS(stmt, ReturnStmt)

  PRINT_IF_SUBCLASS(stmt, Expr)
  PRINT_IF_SUBCLASS(stmt, DeclRefExpr)
  PRINT_IF_SUBCLASS(stmt, CallExpr)
  PRINT_IF_SUBCLASS(stmt, MemberExpr)
  PRINT_IF_SUBCLASS(stmt, CastExpr)
  PRINT_IF_SUBCLASS(stmt, ImplicitCastExpr)
  PRINT_IF_SUBCLASS(stmt, ParenListExpr)

  PRINT_IF_SUBCLASS(stmt, CXXDefaultArgExpr)
  PRINT_IF_SUBCLASS(stmt, CXXConstructExpr)
  PRINT_IF_SUBCLASS(stmt, CXXDependentScopeMemberExpr)
}


void PrintClangASTNodes::printDeclStmt(clang::DeclStmt const *stmt)
{
  char const *qualifier = "DeclStmt::";

  printDeclGroupRef(qualifier, "DG",
    stmt->getDeclGroup());

  OUT_QATTR_LOC(qualifier, "StartLoc",
    stmt->getBeginLoc());
  OUT_QATTR_LOC(qualifier, "EndLoc",
    stmt->getEndLoc());
}


void PrintClangASTNodes::printCompoundStmt(
  clang::CompoundStmt const *stmt)
{
  char const *qualifier = "CompoundStmt::";

  OUT_QATTR_LOC(qualifier, "LBraceLoc",
    stmt->getLBracLoc());
  OUT_QATTR_LOC(qualifier, "RBraceLoc",
    stmt->getRBracLoc());

  OUT_QATTR_INT(qualifier, "NumStmts",
    stmt->size());

  for (unsigned i=0; i < stmt->size(); ++i) {
    OUT_QATTR_PTR(qualifier, "Stmt[" << i << "]",
      getStmtIDStr(stmt->body_begin()[i]));
  }
}


void PrintClangASTNodes::printValueStmt(clang::ValueStmt const *stmt)
{
  // There is no new data in a ValueStmt.  It primarily helps classify
  // nodes in the type hierarchy.  It also has ValueStmt::getExprStmt()
  // with some logic for skipping labels and attributes, which should be
  // documented but isn't.
}


void PrintClangASTNodes::printReturnStmt(clang::ReturnStmt const *stmt)
{
  char const *qualifier = "ReturnStmt::";

  OUT_QATTR_STMT(qualifier, "RetExpr",
    stmt->getRetValue());

  OUT_QATTR_DECL(qualifier, "NRVOCandidate",
    stmt->getNRVOCandidate());

  OUT_QATTR_LOC(qualifier, "RetLoc",
    stmt->getReturnLoc());
}


void PrintClangASTNodes::printExpr(clang::Expr const *expr)
{
  OUT_QATTR_STRING("Expr::ExprBits::", "ValueKind",
    exprValueKindStr(expr->getValueKind()));

  OUT_QATTR_STRING("Expr::ExprBits::", "ObjectKind",
    exprObjectKindStr(expr->getObjectKind()));

  OUT_QATTR_STRING("Expr::ExprBits::", "Dependent",
    exprDependenceStr(expr->getDependence()));

  OUT_QATTR_QUALTYPE("Expr::", "TR",
    expr->getType());
}


void PrintClangASTNodes::printDeclRefExpr(clang::DeclRefExpr const *expr)
{
  OUT_QATTR_BITSET("DeclRefExpr::", "DeclRefExprBits",
    (expr->hasQualifier()?
         " HasQualifier" : "") <<
    (expr->hasTemplateKWAndArgsInfo()?
         " HasTemplateKWAndArgsInfo" : "") <<
    (SPY(DeclRefExpr, expr, hasFoundDecl)?
                          " HasFoundDecl" : "") <<
    (expr->hadMultipleCandidates()?
         " HadMultipleCandidates" : "") <<
    (expr->refersToEnclosingVariableOrCapture()?
         " RefersToEnclosingVariableOrCapture" : "") <<
    (expr->isNonOdrUse()?
       stringb(" " << nonOdrUseReasonStr(expr->isNonOdrUse())) :
       string("")));

  // The location of the declaration name itself.
  OUT_QATTR_STRING("DeclRefExpr::", "Loc",
    locStr(expr->getLocation()));

  OUT_QATTR_PTR("DeclRefExpr::", "D",
    getDeclIDStr(expr->getDecl()));

  // This has three components.  Two are redundant with the 'Loc' and
  // 'D' fields, while the third is 'DNLoc'.  But interpreting 'DNLoc'
  // requires knowing what kind of name 'D' has, so we use the
  // DeclarationNameInfo API to help with that.
  printDeclarationNameInfoLocInfo("DeclRefExpr::", "DNLoc",
    expr->getNameInfo());

  // Optional elements represented as trailing objects:

  if (expr->hasQualifier()) {
    OUT_QATTR_STRING("DeclRefExpr::", "NestedNameSpecifierLoc",
      nestedNameSpecifierLocStr(expr->getQualifierLoc()));
  }

  if (SPY(DeclRefExpr, expr, hasFoundDecl)) {
    OUT_QATTR_PTR("DeclRefExpr::", "FoundDecl",
      getDeclIDStr(expr->getFoundDecl()));
  }

  if (expr->hasTemplateKWAndArgsInfo()) {
    OUT_QATTR_STRING("DeclRefExpr::ASTTemplateKWAndArgsInfo::", "TemplateKWLoc",
      locStr(expr->getTemplateKeywordLoc()));

    OUT_QATTR_STRING("DeclRefExpr::ASTTemplateKWAndArgsInfo::", "LAngleLoc",
      locStr(expr->getLAngleLoc()));

    OUT_QATTR_STRING("DeclRefExpr::ASTTemplateKWAndArgsInfo::", "RAngleLoc",
      locStr(expr->getRAngleLoc()));

    OUT_QATTR_STRING("DeclRefExpr::ASTTemplateKWAndArgsInfo::", "NumTemplateArgs",
      expr->getNumTemplateArgs());

    for (unsigned i=0; i < expr->getNumTemplateArgs(); ++i) {
      printTemplateArgumentLoc(
        "DeclRefExpr::",
        stringb("TemplateArgumentLoc[" << i << "]"),
        expr->getTemplateArgs() + i);
    }
  }
}


void PrintClangASTNodes::printCallExpr(clang::CallExpr const *expr)
{
  char const *qualifier = "CallExpr::";

  llvm::ArrayRef<clang::Stmt*> rawSubExprs =
    const_cast<clang::CallExpr *>(expr)->getRawSubExprs();

  // The number of "raw" subexpressions should be either one or two
  // more than the "actual" arguments.  The first one is the callee.
  // The purpose of the second is unknown to me.
  PRINT_ASSERT(rawSubExprs.size() == 1 + expr->getNumArgs() ||
               rawSubExprs.size() == 2 + expr->getNumArgs());

  // There isn't a particularly easy way to spy on this flag, so I will
  // compute it from the publicly available information.
  bool hasPreArg = (rawSubExprs.size() == 2 + expr->getNumArgs());

  // This also is not easy to spy.
  unsigned OffsetToTrailingObjects =
    reinterpret_cast<char const*>(expr->getArgs() - 1 - (hasPreArg?1:0)) -
    reinterpret_cast<char const*>(expr);

  OUT_QATTR_BITSET(qualifier, "CallExprBits",
    (hasPreArg?
      " hasPreArg" : "") <<
    (expr->usesADL()?
         " UsesADL" : "") <<
    (expr->hasStoredFPFeatures()?
               " hasFPFeatures" : ""));

  OUT_QATTR_INT(qualifier, "OffsetToTrailingObjects",
    OffsetToTrailingObjects);

  OUT_QATTR_INT(qualifier, "NumArgs",
    expr->getNumArgs());

  OUT_QATTR_LOC(qualifier, "RParenLoc",
    expr->getRParenLoc());

  OUT_QATTR_STMT(qualifier, "Callee",
    expr->getCallee());

  if (hasPreArg) {
    OUT_QATTR_STMT(qualifier, "PreArg",
      rawSubExprs[1]);
  }

  for (unsigned i=0; i < expr->getNumArgs(); ++i) {
    OUT_QATTR_STMT(qualifier, "Arg[" << i << "]",
      expr->getArg(i));
  }
}


void PrintClangASTNodes::printMemberExpr(clang::MemberExpr const *expr)
{
  char const *qualifier = "MemberExpr::";

  #define MEMBEREXPR_BITS_FLAG(flag) \
    (SPY(MemberExpr, expr, flag)? " " #flag : "")

  OUT_QATTR_BITSET(qualifier, "MemberExprBits",
    nonOdrUseReasonStr(expr->isNonOdrUse()) <<
    MEMBEREXPR_BITS_FLAG(IsArrow) <<
    MEMBEREXPR_BITS_FLAG(HasQualifierOrFoundDecl) <<
    MEMBEREXPR_BITS_FLAG(HasTemplateKWAndArgsInfo) <<
    MEMBEREXPR_BITS_FLAG(HadMultipleCandidates));

  #undef MEMBEREXPR_BITS_FLAG

  OUT_QATTR_LOC(qualifier, "OperatorLoc",
    expr->getOperatorLoc());

  // Trailing object #1.
  OUT_QATTR_TODO(qualifier, "MemberExprNameQualifier");

  // Trailing object type #2: ASTTemplateKWAndArgsInfo.
  if (SPY(MemberExpr, expr, HasTemplateKWAndArgsInfo)) {
    // The 'LAngleLoc' and 'RAngleLoc' fields get copied into and
    // printed with the 'TemplateArgumentListInfo' in the next 'if'
    // statement.

    OUT_QATTR_LOC(qualifier << "ASTTemplateKWAndArgsInfo.",
      "TemplateKWLoc",
        expr->getTemplateKeywordLoc());

    OUT_QATTR_INT(qualifier << "ASTTemplateKWAndArgsInfo.",
      "NumTemplateArgs",
        expr->getNumTemplateArgs());
  }

  // Trailing object type #3: sequence of TemplateArgumentLoc.
  if (expr->hasExplicitTemplateArgs()) {
    clang::TemplateArgumentListInfo list;
    expr->copyTemplateArgumentsInto(list);
    printTemplateArgumentListInfo(qualifier, "TemplateArgumentLocs",
      list);
  }

  OUT_QATTR_PTR(qualifier, "Base",
    getStmtIDStr(expr->getBase()));

  OUT_QATTR_PTR(qualifier, "MemberDecl",
    getDeclIDStr(expr->getMemberDecl()));

  // Check the invariant in the documentation of 'getMemberDecl':
  {
    clang::ValueDecl *md = expr->getMemberDecl();
    PRINT_ASSERT(isa<clang::FieldDecl>(md) ||
                 isa<clang::VarDecl>(md) ||
                 isa<clang::CXXMethodDecl>(md) ||
                 isa<clang::EnumConstantDecl>(md));
  }

  OUT_QATTR_LOC(qualifier, "MemberLoc",
    expr->getMemberLoc());

  printDeclarationNameInfoLocInfo(qualifier, "MemberDNLoc",
    expr->getMemberNameInfo());
}


void PrintClangASTNodes::printCastExpr(clang::CastExpr const *expr)
{
  char const *qualifier = "CastExpr::";

  OUT_QATTR_BITSET(qualifier, "CastExprBits",
    expr->getCastKindName() <<
    (expr->hasStoredFPFeatures()?
               " HasFPFeatures" : ""));

  OUT_QATTR_INT(qualifier, "BasePathSize",
    expr->path_size());

  unsigned i=0;
  for (clang::CXXBaseSpecifier const *base : expr->path()) {
    printCXXBaseSpecifier(qualifier,
      stringb("BasePath[" << (i++) << "]"),
        base);
  }

  OUT_QATTR_STMT(qualifier, "Op",
    expr->getSubExpr());

  OUT_QATTR_DECL(qualifier, "getConversionFunction()",
    expr->getConversionFunction());

  OUT_QATTR_TODO(qualifier, "getStoredFPFeatures())");
}


void PrintClangASTNodes::printImplicitCastExpr(
  clang::ImplicitCastExpr const *expr)
{
  char const *qualifier = "ImplicitCastExpr::";

  OUT_QATTR_BOOL(qualifier, "PartOfExplicitCast",
    expr->isPartOfExplicitCast());

  // Although 'ImplicitCastExpr' has some trailing objects that
  // 'CastExpr' does not necessarily have, the accessors are all on
  // 'CastExpr', so that is where they are printed.
}


void PrintClangASTNodes::printParenListExpr(
  clang::ParenListExpr const *expr)
{
  char const *qualifier = "ParenListExpr::";

  OUT_QATTR_INT(qualifier, "NumExprs",
    expr->getNumExprs());

  for (unsigned i=0; i < expr->getNumExprs(); ++i) {
    clang::Expr const *e = expr->getExpr(i);
    OUT_QATTR_PTR(qualifier, "Expr[" << i << "]",
      getStmtIDStr(e));
  }

  OUT_QATTR_LOC(qualifier, "LParenLoc",
    expr->getLParenLoc());
  OUT_QATTR_LOC(qualifier, "RParenLoc",
    expr->getRParenLoc());
}


static std::string constructionKindStr(
  clang::CXXConstructExpr::ConstructionKind ck)
{
  ENUM_TABLE_LOOKUP_OR_STRINGB_CAST(
    clang::CXXConstructExpr::, ConstructionKind, ck,

    CK_Complete,
    CK_NonVirtualBase,
    CK_VirtualBase,
    CK_Delegating,
  )
}


void PrintClangASTNodes::printCXXDefaultArgExpr(
  clang::CXXDefaultArgExpr const *expr)
{
  char const *qualifier = "CXXDefaultArgExpr::";

  OUT_QATTR_LOC(qualifier, "Loc",
    expr->getUsedLocation());

  OUT_QATTR_BOOL(qualifier, "HasRewrittenInit",
    expr->hasRewrittenInit());

  OUT_QATTR_DECL(qualifier, "Param",
    expr->getParam());

  OUT_QATTR_DECL(qualifier, "UsedContext",
    declFromDC(expr->getUsedContext()));

  OUT_QATTR_STMT(qualifier, "getExpr()",
    expr->getExpr());

  OUT_QATTR_STMT(qualifier, "getRewrittenExpr()",
    expr->getRewrittenExpr());

  OUT_QATTR_STMT(qualifier, "getAdjustedRewrittenExpr()",
    expr->getAdjustedRewrittenExpr());
}


void PrintClangASTNodes::printCXXConstructExpr(
  clang::CXXConstructExpr const *expr)
{
  char const *qualifier = "CXXConstructExpr::";

  OUT_QATTR_LOC(qualifier, "Loc",
    expr->getLocation());

  OUT_QATTR_BITSET(qualifier, "CXXConstructExprBits",
    constructionKindStr(expr->getConstructionKind()) <<
    (expr->isElidable()?
           " Elidable" : "") <<
    (expr->hadMultipleCandidates()?
         " HadMultipleCandidates" : "") <<
    (expr->isListInitialization()?
           " ListInitialization" : "") <<
    (expr->isStdInitListInitialization()?
           " StdInitListInitialization" : "") <<
    (expr->requiresZeroInitialization()?
                 " ZeroInitialization" : ""));

  OUT_QATTR_PTR(qualifier, "Constructor",
    getDeclIDStr(expr->getConstructor()));

  OUT_QATTR_STRING(qualifier, "ParenOrBraceRange",
    sourceRangeStr(expr->getParenOrBraceRange()));

  OUT_QATTR_INT(qualifier, "NumArgs",
    expr->getNumArgs());

  unsigned i=0;
  for (clang::Expr const *arg : expr->arguments()) {
    OUT_QATTR_PTR(qualifier, "Arg[" << (i++) << "]",
      getStmtIDStr(arg));
  }
}


void PrintClangASTNodes::printCXXDependentScopeMemberExpr(
  clang::CXXDependentScopeMemberExpr const *expr)
{
  char const *qualifier = "CXXDependentScopeMemberExpr::";

  // I spy this field rather than using 'getBase()' because that
  // requires it not be implicit, yet it is evidently possible for
  // 'Base' to be non-null even in the implicit case.
  OUT_QATTR_PTR(qualifier, "Base",
    getStmtIDStr(SPY(CXXDependentScopeMemberExpr, expr, Base)));

  OUT_QATTR_QUALTYPE(qualifier, "BaseType",
    expr->getBaseType());

  OUT_QATTR_STRING(qualifier, "QualifierLoc",
    nestedNameSpecifierLocStr(expr->getQualifierLoc()));

  printDeclarationNameInfo(qualifier, "MemberNameInfo",
    expr->getMemberNameInfo());

  // TODO: These trailing objects.
  OUT_QATTR_TODO(qualifier, "ASTTemplateKWAndArgsInfo");
  OUT_QATTR_TODO(qualifier, "TemplateArgumentLocs");
  OUT_QATTR_TODO(qualifier, "FirstQualifierFoundInScope");
}


void PrintClangASTNodes::printAttr(clang::Attr const *attr)
{
  OUT_OBJECT(getAttrIDStr(attr));

  // This is just some incomplete, experimental dabbling.

  OUT_ATTR_INT("AttrKind", attr->getParsedKind());

  OUT_ATTR_STRING("getAttrName()->getName()",
    attr->getAttrName()->getName().str());

  OUT_ATTR_STRING("getSpelling()", attr->getSpelling());

  if (auto da = dyn_cast<clang::DeprecatedAttr>(attr)) {
    OUT_QATTR_STRING("DeprecatedAttr::", "message",
      da->getMessage().str());
  }
}


void PrintClangASTNodes::printNestedNameSpecifier(
  clang::NestedNameSpecifier const *nns)
{
  OUT_OBJECT(getNestedNameSpecifierIDStr(nns));

  // What is actually stored is a private enumeration packed into the
  // low bits of the 'Prefix' pointer.  'getKind()' returns enough
  // information to reconstruct the private value, but at the moment I
  // do not care enough to do so here.
  OUT_ATTR_STRING("getKind()",
    nestedNameSpecifierKindStr(nns->getKind()));

  // Note that the prefix can be nullptr even if the kind is not
  // 'Global' because it describes the *syntax* of a nested name, which
  // need not be absolute.
  OUT_ATTR_JSON("Prefix" << ifLongForm(".ptr"),
    nestedNameSpecifierIDSyntaxJson(nns->getPrefix()));

  switch (nns->getKind()) {
    case clang::NestedNameSpecifier::Identifier:
      // I don't seem to have existing code that prints IdentifierInfos,
      // so for the moment I omit printing it.
      OUT_ATTR_TODO("IdentifierInfo");
      break;

    case clang::NestedNameSpecifier::Namespace:
      OUT_ATTR_DECL("Namespace", nns->getAsNamespace());
      break;

    case clang::NestedNameSpecifier::NamespaceAlias:
      OUT_ATTR_DECL("NamespaceAlias", nns->getAsNamespaceAlias());
      break;

    case clang::NestedNameSpecifier::TypeSpec:
    case clang::NestedNameSpecifier::TypeSpecWithTemplate:
      OUT_ATTR_TYPE("TypeSpec",
        nns->getAsType());
      break;

    case clang::NestedNameSpecifier::Global:
      // No info.
      break;

    case clang::NestedNameSpecifier::Super:
      OUT_ATTR_DECL("RecordDecl", nns->getAsRecordDecl());
      break;

    // No default because this 'switch' is exhaustive.
  }
}


// ----------------- PrintClangASTNodes: final passes ------------------
void PrintClangASTNodes::printFunctionTemplateSpecializationInfo(
  clang::FunctionTemplateSpecializationInfo const *ftsi)
{
  bool isMemberSpecialization =
    ACCESS_PRIVATE_FIELD(
      *ftsi,
      clang::FunctionTemplateSpecializationInfo,
      PointerIntPair_FunctionDecl_1_bool,
      0 /*discrim*/).getInt();

  OUT_OBJECT(getFunctionTemplateSpecializationInfoIDStr(ftsi));

  // I don't think there's any value in printing the address here
  // because these do not show up in the normal AST dump and I have
  // my own unique numeric IDs.

  OUT_ATTR_PTR("Function" << ifLongForm(".ptr"),
    getDeclIDStr(ftsi->getFunction()));

  /*
    Documentation on the bit:

      flag indicating if the function is a member specialization.

    The bits meaning is implied by the documentation on
    'getMemberSpecializationInfo()':

      Get the specialization info if this function template specialization is
      also a member specialization:

    From my perspective, the ground truth here is that the bit controls
    whether an MSI object is present.
  */
  OUT_QATTR_STRING("Function.", "isFuncMemberSpec",
    isMemberSpecialization);

  OUT_ATTR_PTR("Template" << ifLongForm(".ptr"),
    getDeclIDStr(ftsi->getTemplate()));

  OUT_QATTR_STRING("Template.", "specKind",
    templateSpecializationKindStr(ftsi->getTemplateSpecializationKind()));

  // This is not documented as nullable but I'm being defensive.
  OUT_ATTR_STRING("TemplateArguments",
    templateArgumentListOptStr(ftsi->TemplateArguments));

  OUT_ATTR_STRING("TemplateArgumentsAsWritten",
    astTemplateArgumentListInfoOptStr(ftsi->TemplateArgumentsAsWritten));

  OUT_ATTR_STRING("PointOfInstantiation",
    locStr(ftsi->PointOfInstantiation));

  if (isMemberSpecialization) {
    OUT_ATTR_PTR("MemberSpecializationInfo",
      getMemberSpecializationInfoIDStr(ftsi->getMemberSpecializationInfo()));
  }
}


void PrintClangASTNodes::printRedeclarableTemplateDecl_CommonBase(
  clang::RedeclarableTemplateDecl_CommonBase const *common,
  clang::RedeclarableTemplateDecl const *decl)
{
  if (m_config.m_printAddresses) {
    OUT_ATTR_STRING("address", common);
  }

  OUT_ATTR_PTR("associated decl",
    getDeclIDStr(decl));

  char const *qualifier = "CommonBase::";

  llvm::PointerIntPair<clang::RedeclarableTemplateDecl*, 1, bool>
    InstantiatedFromMember =
      SPY(RedeclarableTemplateDecl, common, InstantiatedFromMember);
  OUT_QATTR_PTR(qualifier,
    "InstantiatedFromMember" << ifLongForm(".ptr"),
      getDeclIDStr(InstantiatedFromMember.getPointer()));

  /*
    The documentation of this bit is confusing.  The bit itself says:

      The boolean value indicates whether this template
      was explicitly specialized.

    but the bit is returned by the 'isMemberSpecialization()' method,
    which says:

      Determines whether this template was a specialization of a
      member template.

    From the examples I've studied, it seems the "explicit" part is
    accurate.

    It is also confusing that there are two other bits, on other
    structures, called 'isMemberSpecialization', which seem to have
    fairly different meanings.
  */
  OUT_QATTR_STRING(qualifier << "InstantiatedFromMember.",
    "explicitMemberSpec",
      InstantiatedFromMember.getInt());

  if (uint32_t *lazySpecs =
        SPY(RedeclarableTemplateDecl, common, LazySpecializations)) {
    uint32_t numLazySpecs = lazySpecs? lazySpecs[0] : 0;

    OUT_QATTR_STRING(qualifier, "LazySpecializations[0] (numLazySpecs)",
      numLazySpecs);

    for (uint32_t i=0; i < numLazySpecs; ++i) {
      // TODO: Interpret these integers.
      OUT_QATTR_STRING(qualifier, "LazySpecializations[" << (i+1) << "]",
        lazySpecs[i+1]);
    }
  }
  else {
    OUT_QATTR_STRING(qualifier, "LazySpecializations", "null");
  }

  if (clang::TemplateArgument *injectedArgs =
        SPY(RedeclarableTemplateDecl, common, InjectedArgs)) {
    unsigned numParams = decl->getTemplateParameters()->size();
    for (unsigned i=0; i < numParams; ++i) {
      OUT_QATTR_STRING(qualifier, "InjectedArgs[" << i << "]",
        doubleQuote(templateArgumentStr(injectedArgs[i])));
    }
  }
  else {
    OUT_QATTR_STRING(qualifier, "InjectedArgs", "null");
  }
}


void PrintClangASTNodes::printFunctionTemplateDecl_Common(
  clang::FunctionTemplateDecl_Common const *common)
{
  OUT_OBJECT(getFunctionTemplateDecl_CommonIDStr(common));

  clang::FunctionTemplateDecl const *decl =
    m_mapCommonToFunctionTemplateDecl.at(common);

  printRedeclarableTemplateDecl_CommonBase(common, decl);

  char const *qualifier = "FTD::Common::";

  unsigned i = 0;
  for (clang::FunctionTemplateSpecializationInfo &ftsi :
         common->Specializations) {
    OUT_QATTR_PTR(qualifier, "Specializations[" << i << "]",

      // The 'Specializations' data structure contains pointers, not
      // objects.  But the 'FoldingSetVector' interface exposes an
      // iterator that pretends it stores objects (I think that is a
      // questionable design decision).  So we need to take the
      // address of the iteration variable to get the actual pointer,
      // which is then the key for my maps.
      getOrCreateFunctionTemplateSpecializationInfoIDStr(&ftsi));

    ++i;
  }
  if (i == 0) {
    OUT_QATTR_STRING(qualifier, "Specializations", "empty");
  }
}


void PrintClangASTNodes::printClassTemplateDecl_Common(
  clang::ClassTemplateDecl_Common const *common)
{
  OUT_OBJECT(getClassTemplateDecl_CommonIDStr(common));

  clang::ClassTemplateDecl const *decl =
    m_mapCommonToClassTemplateDecl.at(common);

  printRedeclarableTemplateDecl_CommonBase(common, decl);

  char const *qualifier = "CTD::Common::";

  unsigned i = 0;
  for (clang::ClassTemplateSpecializationDecl &ctsd :
         common->Specializations) {
    OUT_QATTR_PTR(qualifier, "Specializations[" << i << "]",
      getOrCreateDeclIDStr(&ctsd));

    ++i;
  }
  if (i == 0) {
    OUT_QATTR_STRING(qualifier, "Specializations", "empty");
  }

  i = 0;
  for (clang::ClassTemplatePartialSpecializationDecl &ctpsd :
         common->PartialSpecializations) {
    OUT_QATTR_PTR(qualifier, "PartialSpecializations[" << i << "]",
      getOrCreateDeclIDStr(&ctpsd));

    ++i;
  }
  if (i == 0) {
    OUT_QATTR_STRING(qualifier, "PartialSpecializations", "empty");
  }

  OUT_QATTR_QUALTYPE(qualifier, "InjectedClassNameType",
    common->InjectedClassNameType);
}


void PrintClangASTNodes::printMemberSpecializationInfo(
  clang::MemberSpecializationInfo const *msi)
{
  OUT_OBJECT(getMemberSpecializationInfoIDStr(msi));

  OUT_ATTR_PTR("Member",
    getDeclIDStr(msi->getInstantiatedFrom()));

  OUT_ATTR_STRING("TemplateSpecializationKind",
    templateSpecializationKindStr(
      msi->getTemplateSpecializationKind()));

  OUT_ATTR_STRING("PointOfInstantiation",
    locStr(msi->getPointOfInstantiation()));
}


void PrintClangASTNodes::printDependentFunctionTemplateSpecializationInfo(
  clang::DependentFunctionTemplateSpecializationInfo const *dftsi)
{
  OUT_OBJECT(getDependentFunctionTemplateSpecializationInfoIDStr(dftsi));

  OUT_ATTR_INT("NumTemplates",
    dftsi->getNumTemplates());

  OUT_ATTR_INT("NumArgs",
    dftsi->getNumTemplateArgs());

  OUT_ATTR_LOC("AngleLocs.Begin",
    dftsi->getLAngleLoc());
  OUT_ATTR_LOC("AngleLocs.End",
    dftsi->getRAngleLoc());

  for (unsigned i=0; i < dftsi->getNumTemplateArgs(); ++i) {
    printTemplateArgumentLoc(
      "",
      stringb("Arg[" << i << "]"),
      &(dftsi->getTemplateArg(i)));
  }

  for (unsigned i=0; i < dftsi->getNumTemplates(); ++i) {
    OUT_ATTR_DECL("Template[" << i << "]",
      dftsi->getTemplate(i));
  }
}


void PrintClangASTNodes::printFake_CXXRecordDecl_DefinitionData(
  clang::Fake_CXXRecordDecl_DefinitionData const *fakeData)
{
  clang::CXXRecordDeclSpy::CXXRecordDecl_DefinitionData const *defData =
    toRealDD(fakeData);

  OUT_OBJECT(getFake_CXXRecordDecl_DefinitionDataIDStr(fakeData));

  m_os << "  \"flags\": [\n" <<

    // Flags (Width==1) from the Bits.def file.
    #define FIELD(Name, Width, Merge)                     \
      ((Width==1 && defData->Name)?                       \
         stringb("    " << doubleQuote(#Name) << ",\n") : \
         string("")) <<
    #include "clang/AST/CXXRecordDeclDefinitionBits.def"

    // Other flags that are not in that file for some reason.
    #define PR_FLAG(Name)                                 \
      (defData->Name?                                     \
         stringb("    " << doubleQuote(#Name) << ",\n") : \
         string("")) <<

    PR_FLAG(IsLambda)
    PR_FLAG(IsParsingBaseSpecifiers)
    PR_FLAG(ComputedVisibleConversions)
    PR_FLAG(HasODRHash)

    #undef PR_FLAG

    "  ],\n";

  // The bitfield also has six bit set fields that indicate which of
  // several special member functions have some property.  This
  // macro will print one such set.
  #define SPECIAL_MEMBERS_SET(field)                  \
    OUT_ATTR_STRING(#field,                           \
      clang::CXXRecordDeclSpy::specialMemberFlagsStr( \
        defData->field));

  SPECIAL_MEMBERS_SET(UserDeclaredSpecialMembers)
  SPECIAL_MEMBERS_SET(HasTrivialSpecialMembers)
  SPECIAL_MEMBERS_SET(HasTrivialSpecialMembersForCall)
  SPECIAL_MEMBERS_SET(DeclaredNonTrivialSpecialMembers)
  SPECIAL_MEMBERS_SET(DeclaredNonTrivialSpecialMembersForCall)
  SPECIAL_MEMBERS_SET(DeclaredSpecialMembers)

  #undef SPECIAL_MEMBERS_SET

  OUT_ATTR_STRING("ODRHash", defData->ODRHash);
  OUT_ATTR_STRING("NumBases", defData->NumBases);
  OUT_ATTR_STRING("NumVBases", defData->NumVBases);

  OUT_ATTR_STRING("Bases", "TODO");
  OUT_ATTR_STRING("VBases", "TODO");
  OUT_ATTR_STRING("Conversions", "TODO");
  OUT_ATTR_STRING("VisibleConversions", "TODO");

  OUT_ATTR_PTR("Definition",
    getDeclIDStr(defData->Definition));

  OUT_ATTR_STRING("FirstFriend", "TODO");

  if (defData->IsLambda) {
    auto lambdaData =
      static_cast<clang::CXXRecordDeclSpy::
                  CXXRecordDecl_LambdaDefinitionData const *>(defData);

    OUT_ATTR_BITSET("lambda flags",

      " " << lambdaDependencyKindStr(
        static_cast<clang::CXXRecordDecl::LambdaDependencyKind>(
          lambdaData->DependencyKind)) <<

      (lambdaData->IsGenericLambda?
                 " IsGenericLambda" : "") <<

      " " << lambdaCaptureDefaultStr(
        static_cast<clang::LambdaCaptureDefault>(
          lambdaData->CaptureDefault)) <<

      (lambdaData->HasKnownInternalLinkage?
                 " HasKnownInternalLinage" : "") <<

      "");

    OUT_ATTR_STRING("NumCaptures",
      lambdaData->NumCaptures);

    OUT_ATTR_STRING("NumExplicitCaptures",
      lambdaData->NumExplicitCaptures);

    OUT_ATTR_STRING("ManglingNumber",
      lambdaData->ManglingNumber);

    IF_CLANG_17(
      OUT_ATTR_STRING("IndexInContext",
        lambdaData->IndexInContext)  ,
      /*nothing*/);

    OUT_ATTR_STRING("ContextDecl", "TODO");
    OUT_ATTR_STRING("Captures", "TODO");
    OUT_ATTR_STRING("MethodTyInfo", "TODO");
  }
}


void PrintClangASTNodes::printType(clang::Type const *type)
{
  OUT_OBJECT(getTypeIDStr(type));

  OUT_ATTR_STRING("syntax",
    doubleQuote(typeStr(type)));

  {
    // I want to get Type::BaseType, but that is private.  So, use a dirty
    // trick, casting it first to a base class, then down to a wrong
    // derived class that happens to have a public method that does what I
    // want.  This is technically undefined behavior because the second
    // 'static_cast' is invalid, but it should work fine in practice.
    clang::Type const *baseType =
      static_cast<clang::ExtQuals const *>(
        static_cast<clang::ExtQualsTypeCommonBase const *>(type))->
          getBaseType();

    // If I have a 'Type' object, then it contains a pointer to itself as
    // one of its fields.  This is strange since, elsewhere, the clang
    // data structures are so heavily optimized for space.  The comment on
    // its declaration suggests this is here to make access paths for a
    // QualType with an ExtQuals structure more efficient, but wouldn't it
    // make more sense to embed the pointer in ExtQuals then?
    //
    // Anyway, as long as I assert this, there's no need to separately
    // print the BaseType field.
    PRINT_ASSERT(type == baseType);
  }

  {
    // I hypothesize that the canonical type never has qualifiers, which
    // would imply the following invariant.
    clang::QualType canonicalType = type->getCanonicalTypeInternal();
    PRINT_ASSERT(type->isCanonicalUnqualified() ==
                 (canonicalType.getTypePtr() == type));

    // Print the CanonicalType if it is different than 'type'.
    if (!type->isCanonicalUnqualified()) {
      OUT_ATTR_QUALTYPE(shortAndLongForms("Canon", "CanonicalType"),
        canonicalType);
    }
  }

  {
    clang::Type const *desugared = type->getUnqualifiedDesugaredType();
    if (desugared != type) {
      OUT_ATTR_TYPE("getUnqualifiedDesugaredType()",
        desugared);
    }
  }

  if (auto icnt =
        dyn_cast<clang::InjectedClassNameType>(type)) {
    OUT_ATTR_PTR("Decl",
      getOrCreateDeclIDStr(SPY(InjectedClassNameType, icnt, Decl)));
    OUT_ATTR_QUALTYPE("InjectedType",
      icnt->getInjectedSpecializationType());
  }

  else if (auto tst = dyn_cast<clang::TemplateSpecializationType>(type)) {
    OUT_ATTR_STRING("Bits.TypeAlias",
      tst->isTypeAlias());
    OUT_ATTR_STRING("Bits.NumArgs",
      tst->template_arguments().size());

    int i=0;
    for (clang::TemplateArgument const &arg : tst->template_arguments()) {
      OUT_ATTR_STRING("args[" << i << "]",
        templateArgumentAndKindStr(arg));
      if (arg.getKind() == clang::TemplateArgument::Type) {
        OUT_ATTR_QUALTYPE("args[" << i << "].Type",
          arg.getAsType());
      }
      ++i;
    }

    OUT_ATTR_STRING("Template",
      templateNameStr(tst->getTemplateName()));
    OUT_QATTR_PTR("Template.", "TemplateDecl",
      getOrCreateDeclIDStr(tst->getTemplateName().getAsTemplateDecl()));

    if (tst->isTypeAlias()) {
      OUT_ATTR_QUALTYPE("getAliasedType()",
        tst->getAliasedType());
    }

    // This is not always the same as 'getUnqualifiedDesugaredType()'.
    OUT_ATTR_QUALTYPE("desugar()",
      tst->desugar());
  }

  else if (auto parmType = dyn_cast<clang::TemplateTypeParmType>(type)) {
    OUT_ATTR_PTR("TTPDecl",
      getOrCreateDeclIDStr(parmType->getDecl()));
    OUT_ATTR_STRING("Depth",
      parmType->getDepth());
    OUT_ATTR_STRING("Index",
      parmType->getIndex());
    OUT_ATTR_STRING("isParameterPack",
      parmType->isParameterPack());
  }

  else if (auto sttpt = dyn_cast<clang::SubstTemplateTypeParmType>(type)) {
    OUT_ATTR_PTR("AssociatedDecl",
      getOrCreateDeclIDStr(sttpt->getAssociatedDecl()));
    OUT_ATTR_STRING("Index",
      sttpt->getIndex());
    OUT_ATTR_STRING("PackIndex",
      optionalToString(sttpt->getPackIndex(), "none"));
    OUT_ATTR_QUALTYPE("getReplacementType()",
      sttpt->getReplacementType());
  }

  else if (auto tagType = dyn_cast<clang::TagType>(type)) {
    OUT_ATTR_PTR("decl",
      getOrCreateDeclIDStr(SPY(TagType, tagType, decl)));
  }

  else if (auto funcType = dyn_cast<clang::FunctionProtoType>(type)) {
    OUT_ATTR_QUALTYPE("ResultType",
      funcType->getReturnType());

    int i=0;
    for (clang::QualType paramType : funcType->param_types()) {
      OUT_ATTR_QUALTYPE("paramType[" << i << "]",
        paramType);
      ++i;
    }

    // For the moment, only print this in verbose mode so I don't have
    // complaints about my diagrams' graphs being out of date...
    if (m_config.m_printQualifiers) {
      OUT_QATTR_STRING("FunctionProtoType::FunctionTypeBits.",
        "ExceptionSpecType",
          exceptionSpecificationTypeStr(funcType->getExceptionSpecType()));
    }

    // TODO: More bits.
  }

  else if (auto refType = dyn_cast<clang::ReferenceType>(type)) {
    OUT_ATTR_QUALTYPE("PointeeType",
      refType->getPointeeTypeAsWritten());
  }

  else if (auto ptrType = dyn_cast<clang::PointerType>(type)) {
    OUT_ATTR_QUALTYPE("PointeeType",
      ptrType->getPointeeType());
  }

  else if (auto elabType = dyn_cast<clang::ElaboratedType>(type)) {
    OUT_ATTR_STRING("Keyword",
      elaboratedTypeKeywordStr(elabType->getKeyword()));

    OUT_ATTR_JSON("NNS",
      nestedNameSpecifierIDSyntaxJson(elabType->getQualifier()));

    OUT_ATTR_QUALTYPE("NamedType",
      elabType->getNamedType());
  }
}


void PrintClangASTNodes::printAllNodes()
{
  // Put the entire output into a JSON object wrapper.
  m_os << "{\n";

  // Each time we print a node, we may discover and number new nodes,
  // which causes the loop to continue.  It only stops once all nodes
  // have been discovered and printed.
  for (NodeID id = 1; id < m_numbering.m_nextID; ++id) {
    // How many nodes did we print this time?
    int numPrinted = 0;

    // If 'id' is in the domain of the map for 'ClassName', print it as
    // that kind of object.
    #define PRINT_IF_ID_IS(ClassName)                           \
      if (auto itOpt = mapFindOpt(                              \
            m_numbering.m_##ClassName##Map.m_inverseMap, id)) { \
        clang::ClassName const *node = (**itOpt).second;        \
        print##ClassName(node);                                 \
        ++numPrinted;                                           \
      }

    // Check all of the maps.
    SM_PP_MAP_LIST(PRINT_IF_ID_IS,
      CLANG_AST_NODE_NUMBERING_TRACKED_TYPES);

    #undef PRINT_IF_ID_IS

    if (numPrinted == 1) {
      // Don't bother bumping the number of passed assertions here, this
      // is very trivial and not related to the structure of the AST.
    }
    else {
      PRINT_ASSERT_FAIL("numPrinted was " << numPrinted <<
                        " (should be 1) for node ID: " << id);
    }
  }

  closeOpenObjectIf();

  m_os << "}\n";
  m_os.flush();
}


// ------------------------- global functions --------------------------
void dumpClangAST(
  std::ostream &os,
  clang::ASTContext &astContext)
{
  #if 0
    // This version is good for stepping into with the debugger.
    llvm::raw_os_ostream roo(os);
    clang::ASTDumper P(roo, astContext, false /*colors*/);
    P.setDeserialize(true /*Deserialize*/);
    P.Visit(astContext.getTranslationUnitDecl());

  #else
    // This has lighter compile-time dependencies.
    llvm::raw_os_ostream roo(os);
    astContext.getTranslationUnitDecl()->dump(roo);
    os.flush();

  #endif
}


void printClangAST_JSON(
  std::ostream &os,
  clang::ASTContext &astContext)
{
  #if 0
    // This was a one-off test for filing a clang bug.
    runRAVTest(astContext);

  #else
    llvm::raw_os_ostream roo(os);
    astContext.getTranslationUnitDecl()->dump(roo,
      true /*deserialize; ignored*/, clang::ADOF_JSON);
    os.flush();

  #endif
}


int printClangASTNodes(
  std::ostream &os,
  clang::ASTContext &astContext,
  PrintClangASTNodesConfiguration const &config)
{
  ClangASTNodeNumbering numberer;
  numberClangASTNodes(astContext, numberer);

  PrintClangASTNodes printer(os, astContext, config, numberer);
  printer.printAllNodes();

  TRACE1("Passed assertions: " << printer.m_passedAssertions);

  return printer.m_failedAssertions;
}


// EOF
