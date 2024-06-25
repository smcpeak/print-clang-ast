// use-template-via-typedef.cc
// Specialize a template in a typedef before defining it.

// Forward-declare the template.
template <class T>
struct S;                              // SYMLINE(S_decl)

// Cause the specialization declaration to be instantiated.  The
// "instantiated from" declaration is set to the one above.
typedef S<int> S_of_int;

// Now define the template.
template <class T>
struct S {                             // SYMLINE(S_defn)
  T t;
};

// Then instantiate it.  The resulting ClassTemplateSpecializationDecl
// has a getSourceRange() derived from the template's forward
// declaration but a getLocation() derived from the template definition.
S_of_int s;

// EOF
