// ool-defn-tmethod-tclass.cc
// Testcase for --print-ast-nodes that has a templated method defined
// outside its templated class.

// Originally from: https://stackoverflow.com/questions/76917150/how-to-find-sourcelocation-of-the-outer-template-parameter-list-of-a-class-templ

template<class T>
struct my_class
{
  template<class U>
  void foo(U arg);
};

template<class V>
template<class W> void my_class<V>::foo(W arg)
{
}

int main()
{
  return 0;
}

// EOF
