// ct-cont-ct-emspec-of-cspspec.cc
// Partially specialize the inner CT inside the body of the outer CT,
// them define an explicit member specialization of that.

template <class T>
struct Outer {
  template <class U>
  struct Inner;

  // Class scope partial specialization (cspspec).
  template <class V>
  struct Inner<V*>;
};

// Explicit member specialization (emspec) of the cspspec.
template <>
template <class V>
struct Outer<int>::Inner<V*> {
  int t;
  V *u;
};

// Instantiate the emspec.
Outer<int>::Inner<float*> i;

// EOF
