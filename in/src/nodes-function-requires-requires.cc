// nodes-function-requires-requires.cc
// Test an ad-hoc function 'requires' constraint.

// Found at: https://en.cppreference.com/w/cpp/language/requires

template<typename T>
    requires requires (T x) { x + x; } // ad-hoc constraint, note keyword used twice
T add(T a, T b) { return a + b; }

// EOF
