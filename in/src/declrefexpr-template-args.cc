// nodes-declrefexpr-template-args.cc
// Test some cases of DeclRefExpr with template arguments.

template <class T>
T identity(T t)
{
  return t;
}

template <int n>
int retN()
{
  return n;
}

template <class CLASS>
int callDependent()
{
  return CLASS::template tmethod<int>(3);
}

struct HasTMethod {
  template <class T>
  static T tmethod(T t)
  {
    return t;
  }
};

int call(int x)
{
  return
    identity(x) +
    identity<int>(x) +
    retN<7>() +
    callDependent<HasTMethod>();
}


// EOF
