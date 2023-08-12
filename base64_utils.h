#ifndef BASE64_UTILS_H
#define BASE64_UTILS_H
#include <stddef.h>

static inline void replace_wide_quotes_with_fullwidth(wchar_t wc, char **p_new_str); 

// Replace " with ＂
char* replaceDoubleQuotesWithFullwidth(const char *str);

void stripNewlines(char *str);

int extractNumber(const char* str);

// Function to encode a string using Base64
char* base64_encode(const unsigned char *input, int length);

// Function to decode a Base64 encoded string
unsigned char* base64_decode(const char *input, int *length);

// Function to run unit tests
void run_unit_tests();

#endif // BASE64_UTILS_H
