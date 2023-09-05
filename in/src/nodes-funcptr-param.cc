// nodes-funcptr-param.cc
// Printing test for a parameter that is a function pointer.

// Here, 'x' will have:
//   ParmVarDecl::ParmVarDeclBits::ScopeDepthOrObjCQuals: 1
int f(int (*fp)(int x), int y)
{
  return fp(y);
}

// EOF
