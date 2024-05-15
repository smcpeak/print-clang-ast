// spec-of-templ-w-dep-base.cc
// Recognize a specialization of a template with a dependent base.

template<typename T>
class Base { };

// Dependent base 'Base<T>'.
template<typename T>
class Derived1 : public Base<T>
{ };

#if 0
// Non-dependent base 'Base<int>'.
template<typename T>
class Derived2 : public Base<int>
{ };
#endif

Derived1<int> d1;
//Derived2<int> d2;

// EOF
