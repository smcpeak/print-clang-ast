// struct-with-array-member.cc
// Struct with an array member.

struct HasArray {
  int arr[2];
};

#if 0
HasArray getHA()
{
  HasArray ha;
  ha.arr[0] = 3;
  ha.arr[1] = 4;
  return ha;
}

HasArray moveHA(HasArray &&ha)
{
  return ha;
}

int main()
{
  HasArray ha = getHA();
  if (ha.arr[0] + ha.arr[1] != 7) {
    return 1;
  }

  HasArray ha2;
  ha2 = ha;
  if (ha2.arr[0] + ha2.arr[1] != 7) {
    return 2;
  }

  HasArray ha3 = moveHA(static_cast<HasArray&&>(ha));
  if (ha3.arr[0] + ha3.arr[1] != 7) {
    return 3;
  }

  HasArray ha4 = moveHA(HasArray(static_cast<HasArray&&>(ha)));
  if (ha4.arr[0] + ha4.arr[1] != 7) {
    return 4;
  }

  return 0;
}
#endif

// EOF
