#ifndef UTILS_H_
#define UTILS_H_

#include <stdbool.h>

int NUM_BULT_INS();
char *trimWhiteSpace(char *str);
bool isSpecialChar(char ch);
char **special_char_scan(char *line); 
char **parse_whitespace(char *line);
char **parse_arguments(char *line);
char **resolve_paths(char **args);
char **expand_variables(char **args);


#endif