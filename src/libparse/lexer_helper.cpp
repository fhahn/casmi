#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "libparse/lexer_helper.h"

#define BUFFER_LEN 1000

static int row;
static int column;
static int n_token_start = 0;
static int n_token_next_start = 0;
static int n_token_length = 0;
static int buffer_pos = 0;
static bool eof = false;

static char internal_buffer[BUFFER_LEN];

extern FILE* yyin;

char *filename;

void begin_token(const char *token) {
    n_token_start = n_token_next_start;
    n_token_length = strlen(token);
    n_token_next_start += n_token_length;
}

int get_next_char(char b[], int max_len) {
    if (internal_buffer[buffer_pos] == 0) {
        buffer_pos = 0;
        if (fgets(&internal_buffer[0], BUFFER_LEN-1, yyin) == NULL) {
            eof = true;
            return 0;
        }
        row += 1;
        column = 0;
        n_token_next_start = 0;
        return get_next_char(b, max_len);
    } else {
        column += 1;
        buffer_pos += 1;
        b[0] = internal_buffer[buffer_pos-1];
        return 1;
    }
    return -1;
}

void print_error_msg(const char *msg) {
    fprintf(stderr, "error at %s:%d:%d: %s\n", filename, row, column, msg);

    fprintf(stderr, "  ");
    fprintf(stderr, "%s: %s", filename, internal_buffer);

    for (int i=0; i < (4+strlen(filename)); i++) {
        fprintf(stderr, " ");
    }
    for (int i=0; i < n_token_start; i++) {
        fprintf(stderr, ".");
    }

    for (int i=n_token_start; i <= buffer_pos; i++) {
        fprintf(stderr, "^");
    }

    fprintf(stderr, "\n");

}
