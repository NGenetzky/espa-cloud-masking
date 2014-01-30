#ifndef MYSTRING_H
#define MYSTRING_H

#include <stdio.h>
#include "bool.h"

#define MAX_STR_LEN (510)
#define MAX_NUM_VALUE (20)

/* Key string type definition */

typedef struct {
  int key;
  char *string;
} Key_string_t;

/* Key type definition */

typedef struct {
  char *key;               /* Key string */
  size_t len_key;          /* Length of key */
  int nval;                /* Number of values */
  char *value[MAX_NUM_VALUE];  /* Value strings */
  size_t len_value[MAX_NUM_VALUE];  /* Length of value strings */
} Key_t;


char *DupString(char *string);
int GetLine(FILE *fp, char *s);
bool StringParse(char *s, Key_t *key);
int KeyString(char *key, int len, const Key_string_t *key_string, int null_key, 
              int nkey);

#endif

