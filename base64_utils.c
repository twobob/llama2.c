#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <wchar.h>
#include <wctype.h>
#include <locale.h>
#include "base64_utils.h"

static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static inline void replace_wide_quotes_with_fullwidth(wchar_t wc, char **p_new_str) {
    if (wc == L'"' || wc == L'“' || wc == L'”') {
        strcpy(*p_new_str, "＂"); // Replace with full-width double quotes
        *p_new_str += strlen("＂");
    }
}

char* replaceDoubleQuotesWithFullwidth(const char *str) {
    size_t len = strlen(str);
    char *new_str = (char*)malloc(len * 3 + 1);
    char *p_new_str = new_str;

    while (*str != '\0') {
        if ((*str & 0x80) == 0) { // ASCII character (narrow)
            if (*str == '"') {
                strcpy(p_new_str, "＂");
                p_new_str += strlen("＂");
            } else {
                *p_new_str++ = *str;
            }
            str++;
        } else { // Wide character (UTF-8 multibyte)
            mbstate_t mbs;
            memset(&mbs, 0, sizeof(mbs));
            wchar_t wc;
            size_t n = mbrtowc(&wc, str, MB_CUR_MAX, &mbs);
            replace_wide_quotes_with_fullwidth(wc, &p_new_str); // Call the inline function here
            str += n;
        }
    }
    *p_new_str = '\0';

    return new_str;
}



void stripNewlines(char *str) {
    int len = strlen(str);
    for (int i = 0; i < len; i++) {
        if (str[i] == '\n') {
            // Move the rest of the string one position to the left
            for (int j = i; j < len; j++) {
                str[j] = str[j + 1];
            }
            len--; // Decrease the length of the string
            i--; // Re-check the current position since the string has been shifted
        }
    }
}


int extractNumber(const char* str) {
    int num = 0;

    // Traverse the string to extract the number
    for (int i = 0; str[i] != '\0'; i++) {
        if (isdigit(str[i])) {
            num = num * 10 + (str[i] - '0');
        }
    }

    return num;
}

char* base64_encode(const unsigned char *input, int length) {
    char *output = (char *)malloc((length + 2) / 3 * 4 + 1);
    int i, j = 0;

    for (i = 0; i < length; i += 3) {
        int value = (input[i] << 16) + (input[i + 1] << 8) + input[i + 2];
        output[j++] = base64_table[(value >> 18) & 0x3F];
        output[j++] = base64_table[(value >> 12) & 0x3F];
        output[j++] = base64_table[(value >> 6) & 0x3F];
        output[j++] = base64_table[value & 0x3F];
    }

    // Add padding
    if (i - length == 1) {
        output[j - 1] = '=';
    } else if (i - length == 2) {
        output[j - 2] = '=';
        output[j - 1] = '=';
    }

    output[j] = '\0';
    return output;
}

unsigned char* base64_decode(const char *input, int *length) {
    unsigned char *output = (unsigned char *)malloc((strlen(input) / 4) * 3);
    int i, j = 0;

    for (i = 0; i < strlen(input); i += 4) {
        int value = (strchr(base64_table, input[i]) - base64_table) << 18;
        value += (strchr(base64_table, input[i + 1]) - base64_table) << 12;
        value += (strchr(base64_table, input[i + 2]) - base64_table) << 6;
        value += (strchr(base64_table, input[i + 3]) - base64_table);

        output[j++] = (value >> 16) & 0xFF;
        output[j++] = (value >> 8) & 0xFF;
        output[j++] = value & 0xFF;
    }

    // Remove padding
    if (input[strlen(input) - 1] == '=') {
        j--;
        if (input[strlen(input) - 2] == '=') {
            j--;
        }
    }

    *length = j;
    return output;
}

void run_base64_unit_tests() {
const char *test_prompt = "Wibbly wobly jelly £$%^%^%& !!! ... Jamie was so excited that she ran to the slide and started to climb up. She was so happy when she reached the top. She looked down and saw her mom smiling at her";
    char *encoded_test_prompt = base64_encode((const unsigned char *)test_prompt, 75);
    int decoded_length;
    unsigned char *decoded_test_prompt = base64_decode(encoded_test_prompt, &decoded_length);

    // Assert that the decoded string matches the first 75 characters of the original prompt
    assert(strncmp((const char *)decoded_test_prompt, test_prompt, 75) == 0);

    printf("Unit test passed!\n");

    free(encoded_test_prompt);
    free(decoded_test_prompt);
}
