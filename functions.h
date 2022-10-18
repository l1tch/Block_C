#ifndef _FUNCTIONS_H___
#define _FUNCTIONS_H___

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

char** str_split(char* a_str, const char a_delim);
void remove_spaces(char* s);
void remove_char(char* s, char c);
int char_count(char* s, char c);
void read_json(char* payload_string, char*** keys, char*** values);
char* sha256(char *s);
char* get_val_by_key( char*** keys, char*** values, char* selection_key );
char *str_replace(char *orig, char *rep, char *with);


#endif
