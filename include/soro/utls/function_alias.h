#pragma once

namespace soro {

// Taken from:
// https://codereview.stackexchange.com/questions/202791/the-perfect-function-alias
#define FUN_ALIAS_SPEC(SPECS, NEW_NAME, ...)                 \
  template <typename... Args>                                \
  SPECS auto NEW_NAME(Args&&... args) noexcept(              \
      noexcept(__VA_ARGS__(std::forward<Args>(args)...)))    \
      ->decltype(__VA_ARGS__(std::forward<Args>(args)...)) { \
    return __VA_ARGS__(std::forward<Args>(args)...);         \
  }
#define FUN_ALIAS(NEW_NAME, ...) FUN_ALIAS_SPEC(inline, NEW_NAME, __VA_ARGS__)

// Taken from:
// https://stackoverflow.com/questions/58279658/c-variadic-macro-for-pairs
// For now this works only for a single template parameter.
// Expand this when the need for more than one parameter comes around.

#define EVAL0(...) __VA_ARGS__
#define EVAL1(...) EVAL0(EVAL0(EVAL0(__VA_ARGS__)))
#define EVAL2(...) EVAL1(EVAL1(EVAL1(__VA_ARGS__)))
#define EVAL3(...) EVAL2(EVAL2(EVAL2(__VA_ARGS__)))
#define EVAL4(...) EVAL3(EVAL3(EVAL3(__VA_ARGS__)))
#define EVAL(...) EVAL4(EVAL4(EVAL4(__VA_ARGS__)))

#define MAP_END(...)
#define MAP_OUT
#define MAP_COMMA ,

#define MAP_GET_END2() 0, MAP_END
#define MAP_GET_END1(...) MAP_GET_END2
#define MAP_GET_END(...) MAP_GET_END1
#define MAP_NEXT0(test, next, ...) next MAP_OUT
#define MAP_NEXT1(test, next) MAP_NEXT0(test, next, 0)
#define MAP_NEXT(test, next) MAP_NEXT1(MAP_GET_END test, next)

#define DECL_TEMPLATE(type, name) type name

#define DECL_TEMPLATE_NAME(type, name) name

#define MAP_TUPLES0(f, x, peek, ...) \
  f x MAP_NEXT(peek, MAP_TUPLES1)(f, peek, __VA_ARGS__)
#define MAP_TUPLES1(f, x, peek, ...) \
  f x MAP_NEXT(peek, MAP_TUPLES0)(f, peek, __VA_ARGS__)
#define MAP_TUPLES(f, ...) \
  EVAL(MAP_TUPLES1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

#define FUN_ALIAS_SPEC2(SPECS, NEW_NAME, OLD_NAME, ...)                 \
  template <MAP_TUPLES(DECL_TEMPLATE, __VA_ARGS__), typename... Args>   \
  SPECS auto NEW_NAME(Args&&... args) noexcept(                         \
      noexcept(OLD_NAME<MAP_TUPLES(DECL_TEMPLATE_NAME, __VA_ARGS__)>(   \
          std::forward<Args>(args)...)))                                \
      ->decltype(OLD_NAME<MAP_TUPLES(DECL_TEMPLATE_NAME, __VA_ARGS__)>( \
          std::forward<Args>(args)...)) {                               \
    return OLD_NAME<MAP_TUPLES(DECL_TEMPLATE_NAME, __VA_ARGS__)>(       \
        std::forward<Args>(args)...);                                   \
  }

#define FUN_ALIAS_TEMPLATE(NEW_NAME, OLD_NAME, ...) \
  FUN_ALIAS_SPEC2(inline, NEW_NAME, OLD_NAME, __VA_ARGS__)

}  // namespace soro