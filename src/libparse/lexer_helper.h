#ifndef CASMI_LEXER_H
#define CASMI_LEXER_H

#define BUFFER_LEN 1000

void begin_token(const char *token);
int get_next_char(char buffer[], int max_len);
void print_error_msg(const char *msg);

#endif
