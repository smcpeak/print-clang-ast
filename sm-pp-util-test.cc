// sm-pp-util-test.cc
// Tests for sm-pp-util.

#include "sm-pp-util.h"                // module under test

#include <string.h>                    // strcmp
#include <assert.h>                    // assert


static void shouldBe(int expect, int actual)
{
  assert(expect == actual);
}


static void test_not()
{
  shouldBe(1, SM_PP_NOT(0));
  shouldBe(0, SM_PP_NOT(1));
  shouldBe(0, SM_PP_NOT(123));
  shouldBe(0, SM_PP_NOT(foobar));
  shouldBe(0, SM_PP_NOT(foo bar));
  shouldBe(0, SM_PP_NOT(foo 1));
}


static void test_bool()
{
  shouldBe(0, SM_PP_BOOL(0));
  shouldBe(1, SM_PP_BOOL(1));
  shouldBe(1, SM_PP_BOOL(123));
  shouldBe(1, SM_PP_BOOL(foobar));
  shouldBe(1, SM_PP_BOOL(foo bar));
}


static void test_if_else()
{
  shouldBe(444, SM_PP_IF_ELSE(0)(333)(444));
  shouldBe(333, SM_PP_IF_ELSE(1)(333)(444));
  shouldBe(333, SM_PP_IF_ELSE(2)(333)(444));
  shouldBe(333, SM_PP_IF_ELSE(foobar)(333)(444));
}


enum E {
  E_ZERO,
  E_ONE,
  E_TWO,
  E_THREE,
};


#define ENUM_TABLE_LOOKUP_ENTRY(enumerator) \
  { enumerator, #enumerator },


#define ENUM_TABLE_LOOKUP(key, enumerators, ...)                 \
  static struct Entry {                                          \
    E m_key;                                                     \
    char const *m_value;                                         \
  } const entries[] = {                                          \
    SM_PP_MAP(ENUM_TABLE_LOOKUP_ENTRY, enumerators, __VA_ARGS__) \
  };                                                             \
                                                                 \
  for (Entry const &e : entries) {                               \
    if (e.m_key == key) {                                        \
      return e.m_value;                                          \
    }                                                            \
  }


static char const *getEName(E key)
{
  ENUM_TABLE_LOOKUP(key,
    E_ZERO,
    E_ONE,
    E_TWO,
    E_THREE
  )

  return "none";
}

// Test that the lookup macro works properly.
static void test_getEName()
{
  assert(0==strcmp("E_ZERO", getEName(E_ZERO)));
  assert(0==strcmp("E_ONE", getEName(E_ONE)));
  assert(0==strcmp("E_TWO", getEName(E_TWO)));
  assert(0==strcmp("E_THREE", getEName(E_THREE)));
  assert(0==strcmp("none", getEName(static_cast<E>(7))));
}


#define QUALIFIED_ENUM_TABLE_LOOKUP_ENTRY(scopeQualifier, enumerator) \
  { scopeQualifier enumerator, #enumerator },


#define QUALIFIED_ENUM_TABLE_LOOKUP(scopeQualifier, EnumType, key, enumerators, ...) \
  static struct Entry {                                                              \
    scopeQualifier EnumType m_key;                                                   \
    char const *m_value;                                                             \
  } const entries[] = {                                                              \
    SM_PP_MAP_WITH_ARG(QUALIFIED_ENUM_TABLE_LOOKUP_ENTRY, scopeQualifier,            \
                       enumerators, __VA_ARGS__)                                     \
  };                                                                                 \
                                                                                     \
  for (Entry const &e : entries) {                                                   \
    if (e.m_key == key) {                                                            \
      return e.m_value;                                                              \
    }                                                                                \
  }


namespace NS {
  enum AnotherEnum {
    AE_Z,
    AE_O,
    AE_T
  };
}

static char const *getAEName(NS::AnotherEnum key)
{
  QUALIFIED_ENUM_TABLE_LOOKUP(NS::, AnotherEnum, key,
    AE_Z,
    AE_O,
    AE_T
  )

  return "none";
}

static void test_getAEName()
{
  assert(0==strcmp("AE_Z", getAEName(NS::AE_Z)));
  assert(0==strcmp("AE_O", getAEName(NS::AE_O)));
  assert(0==strcmp("AE_T", getAEName(NS::AE_T)));
  assert(0==strcmp("none", getAEName(static_cast<NS::AnotherEnum>(7))));
}


// A list that can be re-used as an argument.
#define ARGS (1,2,3)


static void test_map_list()
{
  static struct Entry {
    int n;
  } const entries[] = {
    #define ENTRY(arg) {arg},

    SM_PP_MAP_LIST(ENTRY, ARGS)

    #undef ENTRY
  };

  int sum=0;
  for (Entry const &e : entries) {
    sum += e.n;
  }
  assert(sum==6);
}


static void test_map_list_with_arg()
{
  static struct Entry {
    int first;
    int n;
  } const entries[] = {
    #define ENTRY(first, arg) {first, arg},

    SM_PP_MAP_LIST_WITH_ARG(ENTRY, 7, ARGS)

    #undef ENTRY
  };

  int sum=0;
  for (Entry const &e : entries) {
    sum += e.first + e.n;
  }
  assert(sum == 21 + 6);
}


void test_sm_pp_util()
{
  test_not();
  test_bool();
  test_if_else();
  test_getEName();
  test_getAEName();
  test_map_list();
  test_map_list_with_arg();
}


// EOF
