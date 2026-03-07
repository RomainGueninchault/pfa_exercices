#ifndef __GRADER_H
#define __GRADER_H

#include <printf.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

////////////////////////////////////////////////////////////////
// Color macros
#define TEST_COLOR "35;1"
#define RESET_COLOR "0"
#define PASSED_COLOR "32"
#define FAILED_COLOR "31;1"
#define WARN_COLOR "34;1"

#ifndef NO_COLOR
# define COLOR(__c, __s) ESCAPE(__c) __s ESCAPE(RESET)
# define ESCAPE(__s) "\x1B[" __s##_COLOR "m"
#else
# define COLOR(__c, __s) __s
# define ESCAPE(__s)
#endif

#ifndef PRINTF_BUFFER_SIZE
# define PRINTF_BUFFER_SIZE 4096
#endif

////////////////////////////////////////////////////////////////
// Describe the call that will be checked
// DESCRIBE(__call, __printf_fmt_string, [__printf_arguments]+)
//
// typical usage:
// int actual = DESCRIBE(foo(_.x, _.y),
//                      "foo(%d, %S)", _.x, _.x, _.y);
// See XPRINTF
// (FIXME this uses gcc extension)
#define DESCRIBE(__call, ...)\
  ({ LAST_CALL(__VA_ARGS__); __call ;})


////////////////////////////////////////////////////////////////
// Assertions
////////////////////////////////////////////////////////////////
// Asserting x == y
//
// ASSERT_EXACT     (format,  expected_val, actual_val, tag)
// ASSERT_EXACT_MSG (expected_val, actual_val, msg, [msg_args]+)
//
// format: is the formatting string used to print the values
// tag: will be inserted in the message. Se AS_XXXX macro for predefined msgs
// msg: is a format string that will be printed if assertion fails
// msg_args: are arguments for format string
//
// You can use the special variable last_call_buffer ("%s") to display the
// last described call.

#define ASSERT_EXACT(__fmt, __expected, __actual, __tag)              \
    ASSERT_EXACT_MSG(__expected, __actual,                            \
        "Expecting %s'"__fmt"' but found '"__fmt"'. Called by:\n\t%s",\
        __tag, __expected, __actual, last_call_buffer)

#define ASSERT_EXACT_MSG(__expected, __actual, ...)     \
    do {                                                \
        if ((__expected) != (__actual))                 \
            FAILED(__VA_ARGS__);                        \
        else                                            \
            PASSED();                                   \
    } while (0)

////////////////////////////////////////////////////////////////
// Asserting x~= y
//
// ASSERT_FP        (format,  expected_val, actual_val, precision, tag)
// ASSERT_FP_MSG    (expected_val, actual_val, precision, msg, [msg_args]*)
//
// assertion for floating point values, asserting equality up to a
// precision in absolute value. See ASSERT_EXACT for arguments description

#define ASSERT_FP(__fmt, __expected, __actual, __prec, __tag)           \
    ASSERT_FP_MSG(__expected, __actual, __prec,                         \
        "Expecting %s'"__fmt"', but found '"__fmt"'. Called by:\n\t%s", \
        __tag,  __expected, __actual, last_call_buffer)

#define ASSERT_FP_MSG(__expected, __actual, __prec, ...)    \
    do {                                                    \
        if (fabs((__expected) - (__actual)) > (__prec))     \
            FAILED(__VA_ARGS__);                            \
        else                                                \
            PASSED();                                       \
    } while (0)

// Tags
#define AS_RETURN "as return value "
#define AS_RESULT "as return value "
#define AS_PARAM(__x) "in parameter `"#__x"`, "
#define AS_CONTAINED_BY(__x) __x" to contain "
#define AS_PRINTED "to be printed "
#define AS_RECURSIVE "call to be recursive. "
#define AS_ITERATIVE "call to NOT be recursive. "
#define AS_USED_FUNCTION(__fun) "`"#__fun"` to be used. Thus "

// Can be used with ASSERT_XXX_MSG
#define CALLED_BY  "Called by:\n\t%s", last_call_buffer

// Primitives types
#define ASSERT_INT(__expected, __actual, __tag) \
    ASSERT_EXACT("%d", __expected, __actual, __tag)
#define ASSERT_UINT(__expected, __actual, __tag) \
    ASSERT_EXACT("%u", __expected, __actual, __tag)
#define ASSERT_LONG(__expected, __actual, __tag) \
    ASSERT_EXACT("%ld", __expected, __actual, __tag)
#define ASSERT_ULONG(__expected, __actual, __tag) \
    ASSERT_EXACT("%lu", __expected, __actual, __tag)
#define ASSERT_LONGLONG(__expected, __actual, __tag) \
    ASSERT_EXACT("%lld", __expected, __actual, __tag)
#define ASSERT_ULONGLONG(__expected, __actual, __tag) \
    ASSERT_EXACT("%llu", __expected, __actual, __tag)
#define ASSERT_FLOAT(__expected, __actual, __prec, __tag) \
    ASSERT_FP("%f", __expected, __actual, __prec, __tag)
#define ASSERT_DOUBLE(__expected, __actual, __prec, __tag) \
    ASSERT_FP("%lg", __expected, __actual, __prec, __tag)
#define ASSERT_CHAR(__expected, __actual, __tag)                      \
    ASSERT_EXACT_MSG(__expected, __actual,                        \
        "Expecting %s'%c' (%hhu), but found '%c' (%hhu). Called by:\n\t%s", \
        __tag, __expected, __expected, __actual, __actual, last_call_buffer)

///// Pointers
#define ASSERT_PTR(__expected, __actual, __tag) \
    ASSERT_EXACT("%p", __expected, __actual, __tag)
#define ASSERT_NULL(__actual, __tag) \
    ASSERT_EXACT("%p", NULL, __actual, __tag)
#define ASSERT_NOT_NULL(__actual, __tag)                                    \
    ASSERT_NOT_NULL_MSG(__actual, "Expected not null %s, but found %p. Called by:\n\t%s",\
          __tag, __actual, last_call_buffer)
#   define ASSERT_NOT_NULL_MSG(__actual, ...) \
    do {                                      \
        if (NULL == (__actual))               \
            FAILED(__VA_ARGS__);              \
        else                                  \
            PASSED();                         \
    } while (0)

///// Strings
#   define ASSERT_STRING(__expected, __actual, __tag)                                   \
    do {                                                                                \
        const char *_exp = __expected, *_act = __actual;                                \
        ASSERT_STRING_MSG(_exp, _act,                                                   \
            "Expecting %s:\n\t%-#S (%zu)\nbut found\n\t%-#S (%zu)\nCalled by:\n\t%s",   \
            __tag, 1, &_exp, strlen(_exp),                                              \
            1, &_act, strnlen(_act, strlen(_exp) + 10), last_call_buffer);              \
    } while (0)

#   define ASSERT_STRING_MSG(__expected, __actual, ...)           \
    do {                                                          \
        if (memcmp(__expected, __actual, strlen(__expected) + 1)) \
            FAILED(__VA_ARGS__);                                  \
        else                                                      \
            PASSED();                                             \
    } while (0)

// Booleans
#   define ASSERT_FALSE(__actual, __tag) \
    ASSERT_BOOL(0, __actual, __tag)
#   define ASSERT_TRUE(__actual, __tag) \
    ASSERT_BOOL(1, __actual, __tag)
#   define ASSERT_BOOL(__expected, __actual, __tag)                                                 \
   ASSERT_EXACT_MSG(!!__expected, !!__actual, "Expecting %s%s, but found %d. Called by:\n\t%s", \
            __tag, ((__expected) ? "true (non zero)" : "false (zero)" ), __actual, last_call_buffer)

// This one is stil there only for compatibility with fred's & george's legacy code
#   define ASSERT_TRUE_MSG(__actual, ...)       \
    ASSERT_BOOL_MSG(1, __actual, __VA_ARGS__)

#   define ASSERT_BOOL_MSG(__expected, __actual, ...) \
    do {                                              \
        if (as_bool(__expected) != as_bool(__actual)) \
            FAILED(__VA_ARGS__);                      \
        else                                          \
            PASSED();                                 \
    } while (0)

// Structures
#   define ASSERT_STRUCT(__sri, __struct_comparator, __expected, __actual, __tag)               \
    ASSERT_STRUCT_MSG(__struct_comparator, __expected, __actual,                                \
            "Expecting %s:\n\t%-"#__sri"@\nbut found:\n\t%-"#__sri"@\nCalled by:\n\t%s",        \
            __tag, 1, (__expected), 1, (__actual), last_call_buffer)
#   define ASSERT_STRUCT_MSG(__struct_comparator, __expected, __actual, ...)    \
    do {                                                                        \
        if (!(((__expected) == (__actual)) ||                                   \
            ((__actual) && (__expected) &&                                      \
             (__struct_comparator)(                                             \
                 (__expected), (__actual) ))))                                  \
            FAILED(__VA_ARGS__);                                                \
        else                                                                    \
            PASSED();                                                           \
    } while (0)

// Arrays
#define ASSERT_ARRAY(__size, __expected, __actual, __fmt, __tag)                          \
  do {                                                                                    \
    int __idx = array_compare(compare_memory,                                             \
            __expected, __actual, __size, sizeof((__expected)[0]), 0);                    \
    if (__idx != -1)                                                                      \
      FAILED("Expecting %s"__fmt" at index %d, but found "__fmt". Called by:\n\t%s",      \
          __tag, (__expected)[__idx], __idx, (__actual)[__idx], last_call_buffer);        \
    else                                                                                  \
      PASSED();                                                                           \
  } while (0)
#define ASSERT_STRUCT_ARRAY(__sri, __comparator, __size, __expected, __actual, __tag)                   \
  do {                                                                                                  \
    int __idx = array_compare(enlarge_compartor(__comparator),                                          \
            __expected, __actual, __size, sizeof((__expected)[0]), 0);                                  \
    if (__idx != -1)                                                                                    \
      FAILED("Expecting %s:\n\t%-"#__sri"@\nat index %d, but found\n\t%-"#__sri"@\nCalled by:\n\t%s",   \
          __tag, 1, &(__expected)[__idx], __idx, 1, &(__actual)[__idx], last_call_buffer);              \
    else                                                                                                \
      PASSED();                                                                                         \
  } while (0)

#define ASSERT_ARRAY_FP(__size, __expected, __actual, __prec, __fmt, __tag)               \
  do {                                                                                    \
    int __idx = array_compare(compare_memory_prec,                                        \
            __expected, __actual, __size, sizeof((__expected)[0]), __prec);               \
    if (__idx != -1)                                                                      \
      FAILED("Expecting %s"__fmt" at index %d, but found "__fmt". Called by:\n\t%s",      \
          __tag, (__expected)[__idx], __idx, (__actual)[__idx], last_call_buffer);        \
    else                                                                                  \
      PASSED();                                                                           \
  } while (0)
#define ASSERT_ARRAY_UNORDERED(__size, __expected, __actual, __fmt, __tag)                \
  do {                                                                                    \
    void *__m1[__size];                                                                   \
    int __m = array_compare_unordered_missings(compare_memory,                            \
            __expected, __actual, __size, sizeof((__expected)[0]), __m1, NULL);           \
    if (__m != 0)                                                                         \
      FAILED("Expecting %s"__fmt" and %d other elements to be found in array."            \
              " Called by:\n\t%s",                                                        \
          __tag, (__expected)[0], __m-1, last_call_buffer);                               \
    else                                                                                  \
      PASSED();                                                                           \
  } while (0)
#define ASSERT_STRUCT_ARRAY_UNORDERED(__comparator, __size, __expected, __actual, __tag)\
  do {                                                                                  \
    void *__m2[__size];                                                                 \
    int __m = array_compare_unordered_missings(__comparator,                            \
            __expected, __actual, __size, sizeof((__expected)[0]), __m1, NULL);         \
    if (__m != 0)                                                                       \
      FAILED("Expecting %s:\n\t%@\nto be found in array\n\t%@. Called by:\n\t%s",       \
          __tag, __m, __m1, __size, __actual, last_call_buffer);            \
    else                                                                                \
      PASSED();                                                                         \
  } while (0)

// Enum
// (a function name name_of_##enum should be provided, cf `ENUM_RENDERER`)
#define ASSERT_ENUM(__enum, __expected, __actual, __tag) \
    ASSERT_EXACT_MSG(__expected, __actual,                \
        "Expecting %s<%s> but found <%s>. Called by:\n\t%s",\
        __tag, name_of_##__enum(__expected), name_of_##__enum(__actual), last_call_buffer)

////////////////////////////////////////////////////////////////
// Asserting that a function has been implemented
//
// ASSERT_IMPLEMENTED(function_name)

#define PASSED() assert_passed()
#define ASSERT_IMPLEMENTED(__fun)                                           \
    do {                                                                    \
        if (((void*)__fun) == NULL || ((void*)__fun) == ((void*)__##__fun)) \
            FAILED("Not implemented: %s", #__fun);                          \
        else                                                                \
            PASSED();                                                       \
    } while(0)

#define ASSERT_IMPLEMENTED_SILENT(__fun)                                      \
    do {                                                                      \
        if (((void*)__fun) == NULL || ((void*)__fun) == ((void*)__##__fun)) { \
            assert_failed();                                                  \
            return;                                                           \
        } else                                                                \
            PASSED();                                                         \
    } while(0)

#define ASSERT_STILL_PADDED(__len, __tab, __tag)  \
  ASSERT_EXACT_MSG(PAD_VALUE, *(((unsigned char*)__tab) + sizeof((__tab)[0])*__len - 1),    \
          "Buffer overflow %sat index %d. Called by:\n\t%s", __tag, __len, last_call_buffer)

// TODO After here macro have to be checked

#define TRACE(__fun) trace_function(__fun)
#define TRACE_RESULT(__var) struct call_info __var = trace_function(NULL)
#define ASSERT_CALL_DEPTH(__value, __tag)                       \
    do {                                                        \
        struct call_info _call_info = trace_function(NULL);     \
        ASSERT_EXACT_MSG(_call_info.stack_depth, __value,       \
                "Expecting %scall stack should be %d but %d found. Called by:\n\t%s", \
            __tag, __value, _call_info.stack_depth, last_call_buffer); \
    } while(0)

////////////////////////////////////////////////////////////////
// Asserting if a function is recursive or not
//
// ASSERT_NOT_RECURSIVE(function_name, args)
// ASSERT_RECURSIVE(stack_depth, function_name, args)
//
// Ex. : ASSERT_RECURSIVE(4, fibo_rec, 4) asserts that fibo_rec(4)
// makes 4 recursive calls.

#define TRACE_CALL(__var, __fun, ...)          \
  trace_function(__fun);                       \
  __fun(__VA_ARGS__);                          \
  struct call_info __var = trace_function(NULL)

#define ASSERT_NOT_RECURSIVE(__fun, ...)            \
    do {                                            \
        TRACE_CALL(_call_info, __fun, __VA_ARGS__); \
        ASSERT_EXACT_MSG(1, _call_info.stack_depth, \
            #__fun "() should not be recursive.");  \
    } while(0)
#define ASSERT_RECURSIVE(__stack_depth, __fun, ...)             \
    do {                                                        \
        TRACE_CALL(_call_info, __fun, __VA_ARGS__);             \
        ASSERT_EXACT_MSG(__stack_depth, _call_info.stack_depth, \
            #__fun "() should be recursive.");                  \
    } while(0)

////////////////////////////////////////////////////////////////
// Asserting that a call triggers an assertion or exit() with
// some values.
// These macro should never be nested (nor interleaved).
//
// ASSERT_RAISES(call)
// ASSERT_EXIT_WITH(status, call)

#define ASSERT_RAISES(__call) \
  START_BARRIER(trap)         \
    __call ;                  \
    FAILED(#__call " should raise an assertion."); \
  RESCUE_BARRIER              \
    if (user_has_asserted())  \
      PASSED();        \
    else                      \
      FAILED(#__call " should raise an assertion (not an exit)."); \
  END_BARRIER

#define ASSERT_NO_RAISES(__call) \
  START_BARRIER(trap)            \
    __call ;                     \
    PASSED();             \
  RESCUE_BARRIER                 \
    FAILED(#__call " should not raise an assertion (nor exit)."); \
  END_BARRIER

#define ASSERT_EXIT_WITH(__code, __call)   \
  START_BARRIER(trap)                      \
    __call ;                               \
    FAILED(#__call " should exit with status %d (exit not called).", __code); \
  RESCUE_BARRIER                           \
    trap = (trap > 0)?(trap - 1):(trap+1); \
    if (user_has_asserted())               \
      FAILED(#__call " should exit with status %d not an assertion.", __code); \
    else if (__code != trap)               \
      FAILED(#__call " should exit with status %d not %d.", __code, trap); \
    else                                   \
      PASSED();                     \
  END_BARRIER

////////////////////////////////////////////////////////////////
// Assertions on memory allocation
//
// __BEGIN_MEMORY_CHECKING__
// __END_MEMORY_CHECKING__
//
// Define a block where the memory protection mechanisms are
// enabled. At the end, the memory balance is checked with
// ASSERT_MEMORY_BALANCED.
// Ex. :
// __BEGIN_MEMORY_CHECKING__
//    struct fifo * f = __fifo__empty();
//    // the fifo and the link should have been allocated
//    ASSERT_INT(2, (int) get_a_stats().allocated);
//  __END_MEMORY_CHECKING__

#define BEGIN_MEMORY_CHECKING() ({enable_malloc_protector(); get_a_stats();})
#define END_MEMORY_CHECKING() ({ disable_malloc_protector(); get_a_stats(); })

// ASSERT_MEMORY_BALANCED(checkpoint)
//
// checkpoint must have been called with get_a_stats() before. Then
// the macro compares the memory stats between checkpoint and the
// current point, and tests for memory leaks and invalid free(s).

#define ALLOCATED(__s) \
    ((__s).allocated - (__s).freed)

#define ASSERT_MEMORY(__begin, __end, __expected, __on, ...)        \
    do {                                                            \
        long leaks = ALLOCATED(__end) - ALLOCATED(__begin); \
        long faults = __end.fault - __begin.fault;          \
        int has_failed;                                             \
        if (leaks)                                                  \
            MSG(COLOR(FAILED, "Memory leak")": %ld block%s",        \
                    leaks, leaks == 1 ? "": "s");                   \
        if (faults)                                                 \
            MSG(COLOR(FAILED, "Invalid free")" on %ld pointer%s",   \
                    faults, faults == 1 ? "" : "s");                \
        if ((has_failed = (__expected != __on)))                    \
            MSG(__VA_ARGS__);                                       \
        if (has_failed || leaks || faults) {                        \
            FAILED("Called by:\n\t%s", last_call_buffer);           \
            return;                                                 \
        } else                                                      \
            PASSED();                                               \
    } while (0)

////////////////////////////////////////////////////////////////
// FAILED(printf_like_args ...)
//
// Prints an error message to stderr and exits

#ifdef __APPLE__
# define XPRINTF(__file, ...) \
  fxprintf(__file, domain, NULL, __VA_ARGS__)
# define LAST_CALL(...) \
  sxprintf(last_call_buffer, PRINTF_BUFFER_SIZE, domain, NULL, __VA_ARGS__)
  extern printf_domain_t domain;
#else
# define XPRINTF(__file, ...) \
  fprintf(__file, __VA_ARGS__)
# define LAST_CALL(...) \
  snprintf(last_call_buffer, PRINTF_BUFFER_SIZE, __VA_ARGS__)
#endif

#define MSG(...)                                          \
    do {                                                  \
        XPRINTF(stderr, COLOR(WARN, "In %s: "), __func__);\
        XPRINTF(stderr, __VA_ARGS__);                     \
        XPRINTF(stderr, "\n");                            \
        fflush(stderr);                                   \
    } while (0)

#define FAILED(...)       \
    do {                  \
        MSG(__VA_ARGS__); \
        assert_failed();  \
        return;           \
    } while (0)

////////////////////////////////////////////////////////////////
// Define tests cases for a test `test_name`
//
// TEST_CASES(test_name, struct_definition, ...)
// {
//    {case1},
//    {case2},
//    ...
// };
//
// Define tests cases with the same struct_definition than `other_test_name`
// TEST_CASES_LIKE(test_name, other_test_name)
//
// Define an empty test case
// NO_TEST_CASE(test_name)
// You may use UNUSED_TEST_CASES inside your function to mute -Wextra
//
// Reuse the same tests cases than another test. Note: However due to some
// pre-processor limitation, you'll have to use the TEST macro an use
// `other_test_name` as data.
// SAME_TEST_CASES(test_name, other_test_name)
//
// Define a test harness for `test_name`, the function `function_name` should
// be implemented in order to perform all tests
// inside FOR_EACH_TEST, you can use `_` to refer to the test case.
//
// TEST_FUNCTION(test_name) // where test_name == function_name
// TEST_FUNCTION_WITH(test_name, function_name)
// {
//  FOR_EACH_TEST {
//    ... write your test case here
//  }
// }
//
//
// Ex. : (filling an array with even values 0 2 4 6 8 ...)
//
// TEST_CASES(fill_even, int l)
// { { 0 }, { 1 }, { 5 } };
//
// TEST_FUNCTION(fill_even)
// {
//    int expected[6];
//    for (int j = 0; j < ASIZE(expected); j ++)
//      expected[j] = 2 * j;
//    FOR_EACH_TEST {
//        int result[6];
//
//        PAD_BUFFER(result);
//        DESCRIBE(fill_even(_.l, result),
//                "fill_even(%d, result)", _.l);
//
//        ASSERT_ARRAY(_.l, expected, result, "%d", AS_PARAM(result);
//        ASSERT_STILL_PADDED(_.l + 1, result, AS_PARAM(result));
//    }
//  }

#define _ tests_values[test_idx]

#define FOR_EACH_TEST \
  for (int test_idx = set_tests_count(tests_count); test_idx < tests_count ; test_idx = test_passed()) \
    if (test_each_setup(), 1)

#define UNUSED_TEST_CASES       \
  do { (void) tests_values;     \
  (void) tests_count;           \
  test_each_setup();            \
  } while (0)


#define TEST_CASES(__name, ...)           \
  typedef struct {                        \
      FOR_EACH(__TC_ENTRY, , __VA_ARGS__) \
  } tc_##__name;                          \
  tc_##__name test_ ## __name ## _values[] =

#define NO_TEST_CASE(__name)                \
    TEST_CASES(__name, int unused) {{0}}

#define SAME_TEST_CASES(__name, __other)  \
  typedef tc_##__other tc_##__name

#define TEST_CASES_LIKE(__name, __other)  \
  typedef tc_##__other tc_##__name;       \
  tc_##__name test_##__name##_values[] =

#define __TC_ENTRY(a, v) v;


#define TEST_FUNCTION(...)      TEST_FUNCTION_RELAY(__VA_ARGS__)
#define TEST_FUNCTION_WITH(...) TEST_FUNCTION_WRAPPER(__VA_ARGS__)

#define TEST_FUNCTION_RELAY(__name, ...)                \
      TEST_FUNCTION_WRAPPER(__name, __name, __VA_ARGS__)

#define GLOBAL_SETUP \
    static void __attribute__((constructor)) global_setup()
#define LOCAL_SETUP(...) \
    void test_local_setup(__VA_ARGS__)
#define TEST_SETUP(...) \
    void test_each_setup(__VA_ARGS__)

void test_each_setup();
void test_local_setup();

#define CALL_LOCAL_TEST_SETUP(f, ...) \
      test_local_setup(__VA_ARGS__)

#define TESTED_FUNCTION(__fun, ...)                                 \
    do {                                                                      \
        if (((void*)__fun) == NULL || ((void*)__fun) == ((void*)__##__fun)) { \
            assert_failed();                                                  \
            return;                                                           \
        } else                                                                \
            PASSED();                                                         \
    } while(0)


#define TEST_FUNCTION_WRAPPER(__name, ...)                                          \
  ; /* this UGLY semi-colon is here to avoid the forgotten ones after test cases */ \
  static void test_##__name(int tests_count, tc_##__name tests_values[]);           \
  static void test_##__name##_wrapper(int tests_count, tc_##__name tests_values[]) {\
      TESTED_FUNCTION(__VA_ARGS__);                                                 \
      CALL_LOCAL_TEST_SETUP(__VA_ARGS__);                                           \
      test_##__name(tests_count, tests_values);                                     \
  }                                                                                 \
  static void test_##__name(int tests_count, tc_##__name tests_values[])

////////////////////////////////////////////////////////////////
// Buffers helpers
// Pad a buffer with repeated 0x5a, up to the last element which is padded by 0
//
// PAD_BUFFER(buffer) // Buffer should be an array
// PAD_DYNAMIC_BUFFER(buffer, size) // Buffer could be anything dereferenceable
#define PAD_VALUE 0x5a
#define PAD_BUFFER(__buffer)                                    \
    memset(__buffer, PAD_VALUE, sizeof(__buffer))
#define PAD_DYNAMIC_BUFFER(__buffer, __len)                     \
    memset(__buffer, PAD_VALUE, (__len * sizeof((__buffer)[0])))

////////////////////////////////////////////////////////////////
// STRUCT_RENDERER(struct_name, var_name, string, fields...)
// Configures a printer for a given struct name.
// The var_name must be the one used inside the fields.
//
// Ex. :
//
// SIMPLE_STRUCT_RENDERER(iris, i, "{ %s, %d, %d, %d, %d }",
//                 iris_gender[i->gender],
//                 i->sepal_length, i->sepal_width,
//                 i->petal_length, i->petal_width);
// or it's custom alternative
// STRUCT_RENDERER(iris, i, stream)
// {
//      return XPRINTF(stream, "{ %s, %d, %d, %d, %d }",
//                     iris_gender[i->gender],
//                     i->sepal_length, i->sepal_width,
//                     i->petal_length, i->petal_width);
// }
// ...
// STRUCT_RENDERERS(iris)
//
// Then the renderer can be used with something like :
//
// ASSERT_STRING_MSG(expected, actual, "print_iris(%-0@)", 1, &exemples[i]);
//
// The `0` is the struct_renderer used in the order declared by STRUCT_RENDERERS
// The `1` is the size of the array to display.
// The `-` disable the `{}` around items.

typedef int (*printf_renderer)(FILE*, const void*, const void*);
typedef const char *(*enum_renderer)(int);

struct struct_renderer {
    size_t item_size;
    printf_renderer render;
};

extern struct struct_renderer struct_renderers[];
extern enum_renderer enum_renderers[];

int print_escaped_string(FILE *stream, const char** data, const void *unused);

#define SIMPLE_STRUCT_RENDERER(__struct, __var, __fmt, ...)     \
    STRUCT_RENDERER(__struct, __var, stream)                    \
    {                                                           \
        (void)unused;\
        return XPRINTF(stream, __fmt, __VA_ARGS__);             \
    }
#define ALIASED_SIMPLE_STRUCT_RENDERER(__name, __struct, __var, __fmt, ...) \
    ALIASED_STRUCT_RENDERER(__name, __struct, __var, stream)                \
    {                                                                       \
        (void)unused;                                                       \
        return XPRINTF(stream, __fmt, __VA_ARGS__);                         \
    }

#define STRUCT_RENDERER(__struct, __var, __file)                \
    ALIASED_STRUCT_RENDERER(__struct, __struct, __var, __file)

#define ALIASED_STRUCT_RENDERER(__name, __struct, __var, __file)                                \
    static int render_##__name(FILE* __file, const struct __struct *__var, const void *unused); \
    struct struct_renderer __name##_renderer = {                                                \
        .item_size = sizeof(struct __struct),                                                   \
        .render = (printf_renderer)render_##__name,                                             \
    };                                                                                          \
    static int render_##__name(FILE* __file, const struct __struct *__var, const void *unused)

#define STRUCT_RENDERERS(...)                                   \
void initialize_struct_renderers()                              \
{                                                               \
    struct struct_renderer *renderer = struct_renderers;        \
    FOR_EACH(__STRUCT_RENDERERS, X, __VA_ARGS__)                \
}
#define __STRUCT_RENDERERS(X, __name)   *renderer++ = __name##_renderer;

#define ENUM_RENDERERS(...)                                     \
void initialize_enum_renderers()                                \
{                                                               \
    enum_renderer *renderer = enum_renderers;             \
    FOR_EACH(__ENUM_RENDERERS, X, __VA_ARGS__)                  \
}
#define __ENUM_RENDERERS(X, __name)     \
    *renderer++ = (const char * (*)(int))name_of_##__name;


#define SIMPLE_STRUCT_COMPARATOR(__struct)                      \
  static int compare_##__struct(                                \
      const void *p1, const void *p2) {   \
    return !memcmp(p1, p2, sizeof(struct __struct));            \
  }

#define DEFINE_STRUCT_COMPARATOR(__name, __struct, __v1, __v2)                          \
    static int __name##_real(const struct __struct *__v1, const struct __struct *__v2); \
    static int __name(const void *__v1, const void *__v2) {                             \
        const struct __struct *v1 = __v1, *v2 = __v2;                                   \
        return __name##_real(v1, v2);                                                   \
    }                                                                                   \
    static int __name##_real(const struct __struct *__v1, const struct __struct *__v2)


#define SIMPLE_ENUM_RENDERER(__enum, ...)                            \
    static const char *__enum##_txts[] = {                           \
        FOR_EACH(SIMPLE_ENUM_RENDERER_RELAY,, __VA_ARGS__)           \
    };                                                               \
    ENUM_RENDERER(__enum, ASIZE(__enum##_txts), __enum##_txts, 0);
#   define SIMPLE_ENUM_RENDERER_RELAY(__unused, __x) #__x,


#define CUSTOM_ENUM_RENDERER(__name, __enum, __first, ...)           \
    static const char *__name##_txts[] = {                           \
        FOR_EACH(SIMPLE_ENUM_RENDERER_RELAY,, __first, __VA_ARGS__)    \
    };                                                               \
    NAMED_ENUM_RENDERER(__name, __enum,                              \
            ASIZE(__name##_txts) - __first, __name##_txts, -__first);


#define ENUM_RENDERER(__enum, __max, __labels, __offset) \
    NAMED_ENUM_RENDERER(__enum, __enum, __max, __labels, __offset)

#define NAMED_ENUM_RENDERER(__name, __enum, __max, __labels, __offset) \
  static const char *name_of_##__name(enum __enum e_##__enum)            \
  {                                                                      \
      static char buffer[32] = {};                                       \
      if ((e_##__enum) >= ((signed)__max) || (((signed)e_##__enum) < -((signed)__offset))) {     \
          snprintf(buffer, 32, "enum "#__enum "<%d>", (e_##__enum));     \
          return buffer;                                                 \
      }                                                                  \
      return (__labels)[e_##__enum + __offset];                          \
  }

////////////////////////////////////////////////////////////////
// Builds a test suite containing all the tests in the list and
// execute them one by one.
//
// EXERCICE(prefix, list_of_tests ...)
// a test should be a tuple
//   (test)
// or
//   ("name", fun, tests_case)

#define EXERCICE(__name, ...)                   \
    const struct test exercice[] = {            \
        FOR_EACH(_EL_FUN, __name, __VA_ARGS__)  \
        {.name=NULL}                            \
    };

#define EXERCICE_WITHOUT_TESTS(...)             \
    const struct test exercice[] = {            \
        {.name=NULL}                            \
    };

#define UNUSED_TESTS(...)                       \
    const struct test unused_exercice[] = {     \
        FOR_EACH(_EL_FUN, "", __VA_ARGS__)      \
        {.name=NULL}                            \
    };

#define _EL_FUN(prefix, tuple)  _EL_FUN_RELAY(prefix, (PP_NARG tuple), UNPAR tuple)
#define _EL_FUN_RELAY(prefix, n, ...) __TESTN(n)(prefix, __VA_ARGS__)

#define __TESTN(n) __TEST_N n
#define __TEST_N(n) __TEST##n
#define __TEST1(prefix, fun) __TEST(prefix ":" #fun, fun, fun)
#define __TEST2(prefix, fun, data) __TEST(prefix ":" #data, fun, data)
#define __TEST3(prefix, name, fun, data) __TEST(prefix ":" name, fun, data)

#define __TEST(__name, __fun, __data)                           \
  { __name, (testfun_t)test_##__fun##_wrapper,                  \
    ASIZE(test_##__data##_values), &test_##__data##_values },

////////////////////////////////////////////////////////////////
// PROVIDED_FUNCTION(return_type, function_name, [args_types ...])
//
// Defines a function named function_name that can be used within the
// code and the tests. The goal is for the student to be able to use
// the function even if he has not implemented it. If not implemented,
// the weak function will return false with ASSERT_IMPLEMENTED. To use
// the function in the tests, it suffices to prefix it with '__'.

#ifdef __APPLE__
#   ifdef ANSWER
#   define PROVIDED_FUNCTION(__ret, __name, ...) \
      extern __ret __name(__VA_ARGS__); \
      static __ret __##__name(__VA_ARGS__)
#   else
#   define PROVIDED_FUNCTION(__ret, __name, ...) \
      static __ret __##__name(__VA_ARGS__) {exit(1);} \
      static __ret __name(__VA_ARGS__)
#   endif
#else
#   define PROVIDED_FUNCTION(__ret, __name, ...) \
    extern __ret __name(__VA_ARGS__) \
    __attribute__ ((weak, alias(WEAK_DEFINE_RELAY(__##__name)))); \
    static __ret __##__name(__VA_ARGS__)
#   define WEAK_DEFINE_RELAY(__x) #__x
#endif

////////////////////////////////////////////////////////////////
// Public interface

// Test function
typedef void (* const testfun_t) (int, const void*);

// Struct for the tests
struct test {
  const char *name;     // test name
  const testfun_t fun;  // test function
  int len;              // length of the parameters
  const void *param;    // parameters
};

// Call statistics
struct call_info {
  unsigned count;
  unsigned stack_depth;
};

// Allocation statistics
struct a_stats  {
  unsigned long allocated, freed, reallocated;
  unsigned long allocated_size, freed_size;
  unsigned long fault;
};

// Get allocation statistics
struct a_stats get_a_stats();
void enable_malloc_protector();
void disable_malloc_protector();
void free_all();
size_t pointer_info(void*);

// Get call info (for recursion etc..)
struct call_info trace_function(void *sym);

// Main entry point
int grader(const struct test tests[], int argc, char **argv);
void list_tests(const struct test tests[]);

struct printer {
  void (*header)();
  void (*result)(const struct test*, int);
  void (*summary)();
};

const struct test *find_test(const struct test test[], const char *name);
int run_test(const struct test *test, const struct printer *print);
int run_test_group(const struct test test[], const struct printer *print);
int set_tests_count(int tests_count);
int test_passed();
int assert_passed();
int assert_failed();

// User assert support
#define START_BARRIER(__trap)  \
    do {                       \
      int __trap;              \
      reset_user_assert();     \
      jmp_buf _barrier;        \
      if (!(__trap = setjmp(_barrier))) { \
        register_barrier(_barrier);

#define RESCUE_BARRIER         \
      } else {

#define END_BARRIER            \
      }                        \
      release_barrier();       \
    } while(0);

int user_has_asserted();
void reset_user_assert();
void release_barrier();
void register_barrier(jmp_buf checkpoint);

// Grab printf feature
// Note: the size of the buffer is controled by
// -DPRINTF_BUFFER_SIZE=4096
// which has to be set while compiling grader.c (in libexam)
void start_grab_printf();
char* end_grab_printf();

enum grader_mode_t { MAIN, TEST, GRADE };
enum grader_mode_t get_grader_mode();

// Renderers
//void print_test_result(const struct test *test, int status);
//void print_raw_test_result(const struct test *test, int status);

// Helpers
// force positive values to 1 and negative to -1.
int normalize(int v);
// force boolean values to 1 and 0
#define as_bool(__v)  (!!(__v))

size_t strnlen(const char *s, size_t maxlen);

int compare_memory(const void* v1, const void* v2, size_t size, double unused);
int compare_memory_prec(const void* v1, const void* v2, size_t size, double prec);

typedef int (*grader_full_compartor)(const void*, const void*, size_t, double);

#define DO_SEGFAULT() do { int *v = NULL; *v = 1; } while(0)

// Compare two arrays using comparator, and return the differing index
// or -1 if none.
int array_compare(grader_full_compartor comparator,
        const void *a1, const void *a2, size_t n, size_t item_size, double prec);

// Compare two arrays using comparator, and return the number of missing items
// Store missings items in m1 or m2 if provided. They are supposed large enough
int array_compare_unordered_missings_fp(grader_full_compartor comparator,
    const void *a1, const void *a2, size_t items, size_t item_size, double prec,
    void *m1[], void *m2[]);

#define array_compare_unordered(__cmp, __a1, __a2, __n, __size) \
    array_compare_unordered_missings_fp(enlarge_compartor(__cmp), __a1, __a2, __n, __size, 0, NULL, NULL)

#define array_compare_unordered_fp(__cmp, __a1, __a2, __n, __size, __prec) \
    array_compare_unordered_missings_fp(__cmp, __a1, __a2, __n, __size, prec, NULL, NULL)

#define array_compare_unordered_missings(__cmp, __a1, __a2, __n, __size, __m1, __m2) \
    array_compare_unordered_missings_fp(__cmp, __a1, __a2, __n, __size, 0, __m1, __m2)

grader_full_compartor enlarge_compartor(int (*simple_comparator) (const void*, const void*));

extern char last_call_buffer[PRINTF_BUFFER_SIZE];

// Preprocessor helpers
#define ASIZE(__sym) (sizeof(__sym)/sizeof((__sym)[0]))

#define FOR_EACH(action, arg, ...)                                      \
    GET_MACRO(__VA_ARGS__,                                              \
        FE_30,FE_29,FE_28,FE_27,FE_26,FE_25,FE_24,FE_23,FE_22,FE_21,    \
        FE_20,FE_19,FE_18,FE_17,FE_16,FE_15,FE_14,FE_13,FE_12,FE_11,    \
        FE_10, FE_9, FE_8, FE_7, FE_6, FE_5, FE_4, FE_3, FE_2, FE_1)    \
        (action, arg, __VA_ARGS__)

#define PP_NARG(...)                            \
        PP_NARG_(__VA_ARGS__,PP_RSEQ_N())

#define UNPAR(...) __VA_ARGS__

#define  FE_1(WHAT, X, Y)      WHAT(X, Y)
#define  FE_2(WHAT, X, Y, ...) WHAT(X, Y)  FE_1(WHAT, X, __VA_ARGS__)
#define  FE_3(WHAT, X, Y, ...) WHAT(X, Y)  FE_2(WHAT, X, __VA_ARGS__)
#define  FE_4(WHAT, X, Y, ...) WHAT(X, Y)  FE_3(WHAT, X, __VA_ARGS__)
#define  FE_5(WHAT, X, Y, ...) WHAT(X, Y)  FE_4(WHAT, X, __VA_ARGS__)
#define  FE_6(WHAT, X, Y, ...) WHAT(X, Y)  FE_5(WHAT, X, __VA_ARGS__)
#define  FE_7(WHAT, X, Y, ...) WHAT(X, Y)  FE_6(WHAT, X, __VA_ARGS__)
#define  FE_8(WHAT, X, Y, ...) WHAT(X, Y)  FE_7(WHAT, X, __VA_ARGS__)
#define  FE_9(WHAT, X, Y, ...) WHAT(X, Y)  FE_8(WHAT, X, __VA_ARGS__)
#define FE_10(WHAT, X, Y, ...) WHAT(X, Y)  FE_9(WHAT, X, __VA_ARGS__)
#define FE_11(WHAT, X, Y, ...) WHAT(X, Y) FE_10(WHAT, X, __VA_ARGS__)
#define FE_12(WHAT, X, Y, ...) WHAT(X, Y) FE_11(WHAT, X, __VA_ARGS__)
#define FE_13(WHAT, X, Y, ...) WHAT(X, Y) FE_12(WHAT, X, __VA_ARGS__)
#define FE_14(WHAT, X, Y, ...) WHAT(X, Y) FE_13(WHAT, X, __VA_ARGS__)
#define FE_15(WHAT, X, Y, ...) WHAT(X, Y) FE_14(WHAT, X, __VA_ARGS__)
#define FE_16(WHAT, X, Y, ...) WHAT(X, Y) FE_15(WHAT, X, __VA_ARGS__)
#define FE_17(WHAT, X, Y, ...) WHAT(X, Y) FE_16(WHAT, X, __VA_ARGS__)
#define FE_18(WHAT, X, Y, ...) WHAT(X, Y) FE_17(WHAT, X, __VA_ARGS__)
#define FE_19(WHAT, X, Y, ...) WHAT(X, Y) FE_18(WHAT, X, __VA_ARGS__)
#define FE_20(WHAT, X, Y, ...) WHAT(X, Y) FE_19(WHAT, X, __VA_ARGS__)
#define FE_21(WHAT, X, Y, ...) WHAT(X, Y) FE_20(WHAT, X, __VA_ARGS__)
#define FE_22(WHAT, X, Y, ...) WHAT(X, Y) FE_21(WHAT, X, __VA_ARGS__)
#define FE_23(WHAT, X, Y, ...) WHAT(X, Y) FE_22(WHAT, X, __VA_ARGS__)
#define FE_24(WHAT, X, Y, ...) WHAT(X, Y) FE_23(WHAT, X, __VA_ARGS__)
#define FE_25(WHAT, X, Y, ...) WHAT(X, Y) FE_24(WHAT, X, __VA_ARGS__)
#define FE_26(WHAT, X, Y, ...) WHAT(X, Y) FE_25(WHAT, X, __VA_ARGS__)
#define FE_27(WHAT, X, Y, ...) WHAT(X, Y) FE_26(WHAT, X, __VA_ARGS__)
#define FE_28(WHAT, X, Y, ...) WHAT(X, Y) FE_27(WHAT, X, __VA_ARGS__)
#define FE_29(WHAT, X, Y, ...) WHAT(X, Y) FE_28(WHAT, X, __VA_ARGS__)
#define FE_30(WHAT, X, Y, ...) WHAT(X, Y) FE_29(WHAT, X, __VA_ARGS__)

#define GET_MACRO(\
          _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, \
          _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
          _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
          NAME,...) NAME
#define PP_NARG_(...) PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N( \
         _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
        _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
        _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
        _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
        _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
        _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
        _61,_62,_63,  N, ...) N
#define PP_RSEQ_N()           63,62,61,60, \
            59,58,57,56,55,54,53,52,51,50, \
            49,48,47,46,45,44,43,42,41,40, \
            39,38,37,36,35,34,33,32,31,30, \
            29,28,27,26,25,24,23,22,21,20, \
            19,18,17,16,15,14,13,12,11,10, \
             9, 8, 7, 6, 5, 4, 3, 2, 1, 0
#endif // __GRADER_H
