#include "prot/prot_const.h"

typedef int (*text_parser_cb)(const char *cmd, const char *start, const char *stop);

typedef struct
{
    const char *cmd;
    const text_parser_cb cb;
} text_cb_t;

text_cb_t text_cb_tab[] = {
    {"dir", 0}};

const int text_cb_len = sizeof(text_cb_tab) / sizeof(*text_cb_tab);