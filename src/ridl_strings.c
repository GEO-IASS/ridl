#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/**
   @file
   This file contains helper routines dealing with strings specific to rIDL.
*/


/**
   Copies a string. Allocates dynamic memory for return value which the caller
   is responsible for freeing.
   
   @return string
   @param[in] s string to copy
*/
char *ridl_copystr(char *s) {
  char *r = (char *)malloc(strlen(s) + 1);
  strcpy(r, s);
  return(r);
}


/**
   Replace `name` with `value` in `text` and return the result in `result`.

   Use it as in the following example:
   @code
     char *s = (char *)malloc(1024);
     char *t = (char *)malloc(1024);
     strcpy(t, "Hello my name is %name and I live in %city!\n");
     ridl_replacestr(s, t, "name", "Mike");
     printf("%s", s);
  
     strcpy(t, s);
     ridl_replacestr(s, t, "city", "Boulder");
     printf("%s", s);
   @endcode
   
   @return 1 if matched, 0 if not
   
   @param[out] result string to return result in
   @param[in] text string to look for `name` in
   @param[in] name string in `text` to replace
   @param[in] value string to replace `name` with
*/
int ridl_replacestr(char *result, char *text, char *name, char *value) {
  char *pos, *original_name, *original_value;
  int i, matches, matched = 0;
  
  original_name = name;
  original_value = value;
  
  while (*text) {
    if (*text == '%') {
      pos = text;
      text++;
      
      matches = 1;
      for (name = original_name; *name; name++, text++) {
        if (*name != *text) {
          matches = 0;
          break;
        }
      }

      if (matches) {
        matched = 1;
        while ((*result++ = *value++))
          ;
        result--;   // remove the null
        value = original_value;
      } else {
        *result++ = *pos;
      }
    } else {
      *result = *text;
      text++;
      result++;
    }
  }
  *result = '\0';
  
  return(matched);
}


/**
   Return the word starting from index start on line. The result is
   dynamically allocated, so should be freed by the calling routine.
   
   @return char * of next word; caller is responsible for freeing
   
   @param[in] line string to search for next word in
   @param[in] start index in line to start looking for next word
   @param[out] search_length length of the string searched for the next word
*/
char *ridl_getnextword(char *line, int start, int *search_length) {
  int i, end, actualStart = start, actualEnd;
  char *result;
  
  for (i = start; i < strlen(line); i++) {
    if (line[i] == ' ') actualStart++; else break;
  }
  
  for (i = actualStart; i < strlen(line); i++) {
    if (line[i] == ' ') break;
  }
  actualEnd = i;
  
  result = (char *)calloc(actualEnd - actualStart + 1, 1);  
  for (i = actualStart; i < actualEnd; i++) {
    result[i - actualStart] = line[i];
  }
  
  *search_length = actualEnd - start;
  
  return(result);
}
