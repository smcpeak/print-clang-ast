// template-param-object-decl.cc
// Example where the AST has a `TemplateParamObjectDecl`.

// https://stackoverflow.com/questions/78867194/what-does-the-name-of-a-templateparamobject-in-clangs-ast-refer-to

struct foo_t
{
  int a = 3;
};

template <foo_t t> struct bar_t
{
  bool is_default = t.a == 3;
};

int
main()
{
    constexpr auto foo = foo_t{};
    bar_t<foo> bar;
    return 0;
}

// EOF
