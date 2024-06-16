// new.cc
// Exercise some cases of `new` not involving classes.

// I do not want to pull in stddef.h in these tests, so ...
typedef unsigned long size_t;

void *operator new (size_t count, int place1);

void f()
{
  // Simplest.
  new int;

  // Array size.
  new int[5];

  // Placement args and initializer.
  new (3) int(7);
}

// EOF
