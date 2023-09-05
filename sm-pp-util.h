// sm-pp-util.h
// Preprocessor utilities.

// There is no .cc file for this module, only this header, although
// there is sm-pp-util-test.cc with some basic tests.

// Based on: http://jhnet.co.uk/articles/cpp_magic

#ifndef SM_PP_UTIL_H
#define SM_PP_UTIL_H


// Concatenate the expansions of 'a' and 'b'.
#define SM_PP_CAT(a,b) SM_PP_PRIVATE_PRIMITIVE_CAT(a, b)

// This one won't expand either of its arguments before substituting.
#define SM_PP_PRIVATE_PRIMITIVE_CAT(a,b) a##b


// Return the second argument.
#define SM_PP_PRIVATE_SECOND(a, b, ...) b


// Expands to 1 if its argument is 'SM_PP_PRIVATE_PROBE()', and 0 if the
// argument is some other single argument.
#define SM_PP_PRIVATE_IS_PROBE(...) SM_PP_PRIVATE_SECOND(__VA_ARGS__, 0)

// Expand to two arguments, where the second is 1.  This is meant for
// use with SM_PP_PRIVATE_IS_PROBE.  The '~' is just a placeholder to
// push the 1 into the second argument position so that
// 'SM_PP_PRIVATE_SECOND' will return it.
#define SM_PP_PRIVATE_PROBE() ~, 1

// Expand to 1 if the argument is 0, and 0 for anything else.  This
// works because 'SM_PP_PRIVATE_NOT_PROBE_0' is a defined macro, so will
// be recognized and expanded to 'SM_PP_PRIVATE_PROBE()', while anything
// else (such as 'SM_PP_PRIVATE_NOT_PROBE_123') will not be defined, and
// hence won't expand.  In the former case, 'SM_PP_PRIVATE_IS_PROBE' (in
// effect) recognizes the argument and expands to 1, while in the latter
// it expands to 0.
//
// We need to use 'SM_PP_CAT' instead of plain '##' here because plain
// '##' would inhibit 'x' from being expanded again, and hence if 'x'
// were another application of (say) the 'SM_PP_NOT' macro (for example,
// because we are expanding the 'SM_PP_BOOL' macro below), then the
// second application would fail to expand.
#define SM_PP_NOT(x) SM_PP_PRIVATE_IS_PROBE(SM_PP_CAT(SM_PP_PRIVATE_NOT_PROBE_, x))
#define SM_PP_PRIVATE_NOT_PROBE_0 SM_PP_PRIVATE_PROBE()


// Expand to 0 if the argument is 0, and 1 for anything else.
#define SM_PP_BOOL(x) SM_PP_NOT(SM_PP_NOT(x))


// This is meant to be used like this:
//
//   SM_PP_IF_ELSE(cond)(thenCase)(elseCase)
//
// The SM_PP_IF_ELSE(cond) part expands to something that, after two
// more macro expansions, preserves either 'thenCase' or 'elseCase'.
#define SM_PP_IF_ELSE(condition) SM_PP_CAT(SM_PP_PRIVATE_IF_PREFIX_, SM_PP_BOOL(condition))

// Output of 'SM_PP_IF_ELSE' that preserves the 'thenCase'.
#define SM_PP_PRIVATE_IF_PREFIX_1(...) __VA_ARGS__ SM_PP_PRIVATE_IF_PREFIX_1_ELSE
#define SM_PP_PRIVATE_IF_PREFIX_1_ELSE(...)

// Output of 'SM_PP_IF_ELSE' that preserves the 'elseCase'.
#define SM_PP_PRIVATE_IF_PREFIX_0(...) SM_PP_PRIVATE_IF_PREFIX_0_ELSE
#define SM_PP_PRIVATE_IF_PREFIX_0_ELSE(...) __VA_ARGS__


// Evaluate something many times in order to enable a form of recursive
// expansion.  The numbers are a little misleading because there is an
// extra evaluation pass after substitution, so really SM_PP_EVAL2
// evaluates its arguments *three* times.
#define SM_PP_EVAL(...) SM_PP_EVAL128(__VA_ARGS__)
#define SM_PP_EVAL128(...) SM_PP_EVAL64(SM_PP_EVAL64(__VA_ARGS__))
#define SM_PP_EVAL64(...) SM_PP_EVAL32(SM_PP_EVAL32(__VA_ARGS__))
#define SM_PP_EVAL32(...) SM_PP_EVAL16(SM_PP_EVAL16(__VA_ARGS__))
#define SM_PP_EVAL16(...) SM_PP_EVAL8(SM_PP_EVAL8(__VA_ARGS__))
#define SM_PP_EVAL8(...) SM_PP_EVAL4(SM_PP_EVAL4(__VA_ARGS__))
#define SM_PP_EVAL4(...) SM_PP_EVAL2(SM_PP_EVAL2(__VA_ARGS__))
#define SM_PP_EVAL2(...) SM_PP_EVAL1(SM_PP_EVAL1(__VA_ARGS__))
#define SM_PP_EVAL1(...) __VA_ARGS__


// Expands to nothing.  This is used to separate a macro name from its
// arguments for one step of evaluation.
#define SM_PP_PRIVATE_EMPTY()

// Expands to 'macro' without allowing expansion of 'macro' itself at
// the place 'SM_PP_PRIVATE_DEFER1' is used.
#define SM_PP_PRIVATE_DEFER1(macro) macro SM_PP_PRIVATE_EMPTY()

// Need another layer...
#define SM_PP_PRIVATE_DEFER2(macro) macro SM_PP_PRIVATE_EMPTY SM_PP_PRIVATE_EMPTY()()


// Return the first argument:
//
//   SM_PP_PRIVATE_FIRST(1, 2, 3) -> 1
//
#define SM_PP_PRIVATE_FIRST(a, ...) a


// Expand to 1 if the first argument is empty.
//
// This works by first throwing away all but the first argument, then
// juxtaposing 'SM_PP_PRIVATE_END_OF_ARGUMENTS' with that first argument
// and following parens.  If the first argument is empty, we expand
// 'SM_PP_PRIVATE_END_OF_ARGUMENTS()' to 0, and 'SM_PP_NOT' turns it
// into 1.  Otherwise, the first argument and macro parens can expand or
// not, but something will still be there, so 'SM_PP_NOT' turns it into
// 0.
//
// However, if the first argument ends with the name of a function-like
// macro that takes a non-zero number of arguments, this will cause an
// error...
#define SM_PP_PRIVATE_FIRST_IS_EMPTY(...) \
  SM_PP_NOT(SM_PP_PRIVATE_FIRST(SM_PP_PRIVATE_END_OF_ARGUMENTS __VA_ARGS__)())

#define SM_PP_PRIVATE_END_OF_ARGUMENTS() 0


// The core of the map routine that applies 'macro' to all of its
// arguments.  This has to be wrapped in an eval.
#define SM_PP_PRIVATE_MAP_IMPL(macro, first, ...)                                       \
  SM_PP_IF_ELSE(SM_PP_PRIVATE_FIRST_IS_EMPTY(first))                                    \
    ()                                                                                  \
    (macro(first) SM_PP_PRIVATE_DEFER2(SM_PP_PRIVATE_MAP_HELPER)()(macro, __VA_ARGS__))

// This gets invoked to generate the recursive call at the right time.
// It's complicated.
#define SM_PP_PRIVATE_MAP_HELPER() SM_PP_PRIVATE_MAP_IMPL


// Apply 'macro' to all of the arguments:
//
//   SM_PP_MAP(foo, 1, 2, 3) -> foo(1) foo(2) foo(3)
//
#define SM_PP_MAP(macro, ...) SM_PP_EVAL(SM_PP_PRIVATE_MAP_IMPL(macro, __VA_ARGS__))


// Remove a layer of parentheses from the argument list:
//
//   SM_PP_EATPARENS(1,2,3) -> 1,2,3
//
#define SM_PP_EATPARENS(...) __VA_ARGS__

// Prepend an element to a parenthesized list:
//
//   SM_PP_PREPEND(1, (2,3)) -> (1,2,3)
//
#define SM_PP_PREPEND(first, ...) (first, SM_PP_EATPARENS __VA_ARGS__)

// Apply a macro to a parentheszied argument list:
//
//   SM_PP_APPLY(foo, (1,2,3)) -> foo(1,2,3)
//
#define SM_PP_APPLY(macro, arglist) macro arglist

// Invoke 'macro' on each element of parenthesized 'arglist':
//
//   SM_PP_MAP_LIST(foo, (1,2,3)) -> foo(1) foo(2) foo(3)
//
#define SM_PP_MAP_LIST(macro, arglist) \
  SM_PP_APPLY(SM_PP_MAP, SM_PP_PREPEND(macro, arglist))


// Like 'SM_PP_MAP', but also pass 'arg' as the first argument to every
// invocation of 'macro'.
#define SM_PP_PRIVATE_MAP_WITH_ARG_IMPL(macro, arg, first, ...)                          \
  SM_PP_IF_ELSE(SM_PP_PRIVATE_FIRST_IS_EMPTY(first))                                     \
    ()                                                                                   \
    (macro(arg, first)                                                                   \
     SM_PP_PRIVATE_DEFER2(SM_PP_PRIVATE_MAP_WITH_ARG_HELPER)()(macro, arg, __VA_ARGS__))

#define SM_PP_PRIVATE_MAP_WITH_ARG_HELPER() SM_PP_PRIVATE_MAP_WITH_ARG_IMPL


// Apply 'macro' to all of the arguments, with an initial 'arg':
//
//   SM_PP_MAP_WITH_ARG(foo, 0, 1, 2, 3) -> foo(0,1) foo(0,2) foo(0,3)
//
#define SM_PP_MAP_WITH_ARG(macro, arg, ...) \
  SM_PP_EVAL(SM_PP_PRIVATE_MAP_WITH_ARG_IMPL(macro, arg, __VA_ARGS__))


// Apply 'macro' to all elements of the parenthesized 'arglist', with an
// initial 'arg':
//
//   SM_PP_MAP_LIST_WITH_ARG(foo, 0, (1, 2, 3)) -> foo(0,1) foo(0,2) foo(0,3)
//
#define SM_PP_MAP_LIST_WITH_ARG(macro, arg, arglist) \
  SM_PP_APPLY(SM_PP_MAP_WITH_ARG, SM_PP_PREPEND(macro, SM_PP_PREPEND(arg, arglist)))


// Tests, defined in sm-pp-util-test.cc.
void test_sm_pp_util();


#endif // SM_PP_UTIL_H
