// catch-all.cc
// Exercise handling of `catch (...)` and `throw;`.

void foo()
{
  try {
  }
  catch (...) {
    throw;
  }
}

// EOF
