// default-arg-callee.cc
// Default argument involving a function call.

int makeInt();

void f1(int x = makeInt());

void f2()
{
  // The point of this test is that we should see a call to 'makeInt()'
  // here.
  f1();
}

void f3()
{
  f1(makeInt());
}

// EOF
