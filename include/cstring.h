#ifndef CSTRING_H
#define CSTRING_H

#include <arena.h>
#include <stddef.h>

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#include <stdbool.h>
#elif !defined(__cplusplus) && !defined(__bool_true_and_false_are_defined)
typedef int bool;
#define true 1
#define false 0
#define __bool_true_and_false_are_defined 1
#endif

typedef struct String String;

/* Creates a String object from c-string (char *) */
String *string_from(Arena *arena, char *str);

/* Creates a String object from c-string (char *) with specified length*/
String *string_from_len(Arena *arena, size_t len, char *str);

/* Creates a deep copy of an existing String object */
String *string_clone(Arena *arena, String *str);

/* Concatinates n string objects */
String *string_concat(Arena *arena, int n, ...);

/* Returns the length of the string */
int string_len(String *str);

/* Returns a substring from a String where `begin` is inclusive but `end` is
 * exclusive */
String *string_sub(Arena *arena, String *str, int begin, int end);

/* Returns the c-string (char *) from the String object */
char *string(String *st);

/* Concatinates n c-strings (char *) */
String *string_concat_cstr(Arena *arena, int n, ...);

/* Trims all leading and trailing white-spaces from the String */
String *string_trim(Arena *arena, String *str);

/* Takes String input from console */
String *string_get(Arena *arena);

/* Converts all lower-case characters in a string to upper-case*/
char *string_upper(Arena *arena, String *str);

/* Converts all upper-case characters in a string to lower-case*/
char *string_lower(Arena *arena, String *str);

/* Compares each corresponding character of two null-terminated strings without
 * case sensitivity*/
bool string_comp(struct String *s1, struct String *s2);

/* Compares each corresponding character of two null-terminated strings with
case sensitivity*/
bool string_comp_case(struct String *s1, struct String *s2);
#endif // CSTRING_H
