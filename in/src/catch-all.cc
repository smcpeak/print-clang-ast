// catch-all.cc
// Exercise handling of `catch (...)`.

int foo()
{
  try {
  }
  catch (...) {
    return 2;
  }

  return 0;
}

// EOF
