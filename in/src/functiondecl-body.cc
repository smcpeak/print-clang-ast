// functiondecl-body.cc
// Exercise FunctionDecl::Body.

// No body.  Part of the point of the test is we see a null Body here
// even though 'getBody' would retrieve the one associated with the next
// redeclaration.
int f();

// Has body.
int f() { return 2; }

// No body.
int f();

// EOF
