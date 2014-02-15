#ifndef CASMI_LEXER_H
#define CASMI_LEXER_H

#define BUFFER_LEN 1000

void begin_token(char *token);
int get_next_char(char buffer[], int max_len);

#endif
