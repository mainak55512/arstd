#ifndef CERROR_H
#define CERROR_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
  int code;
  const char *name;
  const char *message;
} cerror_t;

typedef struct {
  const char *name;
  const char *message;
} cerror_entry_t;

#define CERROR_OK                                                              \
  ((cerror_t){.code = 0, .name = "ERR_OK", .message = "Success"})
#define CERROR_DEFAULT                                                         \
  ((cerror_t){                                                                 \
      .code = -1, .name = "ERR_DEFAULT", .message = "An error occurred"})

void cerror_register(const cerror_entry_t *entries, size_t count);
#define CERROR_REGISTER(list)                                                  \
  cerror_register((list), sizeof(list) / sizeof((list)[0]))

cerror_t cerror(const char *name);
const char *cerror_strerror(cerror_t err);
const char *cerror_err_name(cerror_t err);
void cerror_panic(cerror_t err, const char *msg, const char *file, int line);

#define CERROR_RESULT(T, Name)                                                 \
  typedef struct {                                                             \
    bool is_ok;                                                                \
    T ok;                                                                      \
    cerror_t err;                                                              \
  } Name;                                                                      \
  static inline T Name##_unwrap_(Name res, const char *file, int line) {       \
    if (!res.is_ok) {                                                          \
      cerror_panic(res.err, NULL, file, line);                                 \
    }                                                                          \
    return res.ok;                                                             \
  }                                                                            \
  static inline T Name##_expect_(Name res, const char *msg, const char *file,  \
                                 int line) {                                   \
    if (!res.is_ok) {                                                          \
      cerror_panic(res.err, msg, file, line);                                  \
    }                                                                          \
    return res.ok;                                                             \
  }                                                                            \
  static inline T Name##_unwrap_or_(Name res, T fallback) {                    \
    return res.is_ok ? res.ok : fallback;                                      \
  }                                                                            \
  struct Name##_cerror_guard__

#define OK(ResType, val) ((ResType){.is_ok = true, .ok = (val)})
#define ERR(ResType, e) ((ResType){.is_ok = false, .err = (e)})
#define IS_OK(res) ((res).is_ok)
#define IS_ERR(res) (!(res).is_ok)

#define UNWRAP(ResType, expr) ResType##_unwrap_((expr), __FILE__, __LINE__)
#define EXPECT(ResType, expr, msg)                                             \
  ResType##_expect_((expr), (msg), __FILE__, __LINE__)
#define UNWRAP_OR(ResType, expr, fallback)                                     \
  ResType##_unwrap_or_((expr), (fallback))

#define TRY(out, RetType, ExprType, expr)                                      \
  do {                                                                         \
    ExprType cerror__res = (expr);                                             \
    if (!cerror__res.is_ok) {                                                  \
      return ERR(RetType, cerror__res.err);                                    \
    }                                                                          \
    (out) = cerror__res.ok;                                                    \
  } while (0)

#endif // CERROR_H
