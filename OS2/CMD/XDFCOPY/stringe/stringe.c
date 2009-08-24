#ifndef stringe_c
   #define stringe_c
// version:
// 07/03/97 - added right function
//          - added pos and lastpos
// 07/15/97 - cleaned up some memory management
// 07/18/98 - clean up for creation of LIB functions
#define DWORD long
#define MAX_STR 2048
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

int words (char *source) {
   int i; char *string;
   if (source == NULL)
      i = 0;
   else {
      string = strdup(source);
      if (strtok (string, " ") == NULL)
         i = 0;
      else
         for (i = 1; strtok(NULL, " "); ++i) {};
      free (string);
      }
   return i;
   }

char *word (char *target, char *source, int pos) {
   int i; char *found, *string;
   if (target == NULL)
      return NULL;
   if (source == NULL)
      target[0] = '\0';
   else {
      string = strdup(source);
      found = strtok (string, " ");
      for (i = 1; i < pos; found = strtok(NULL, " "), i++) {};
      if (found == NULL)
         target [0] = '\0';
      else
         strcpy (target, found);
      free (string);
      }
   return target;
   }

char * strtoupper (char *target, char *source) {
   DWORD i;
   if (target == NULL)
      return NULL;
   if (source == NULL)
      target[0] = '\0';
   else {
      for (i = 0; source[i] != '\0'; i++)
         target [i] = toupper(source [i]);
      target[i] = '\0';
      }
   return target;
   }

char * strtolower (char *target, char *source) {
   DWORD i;
   if (target == NULL)
      return NULL;
   if (source == NULL)
      target[0] = '\0';
   else {
      for (i = 0; source[i] != '\0'; i++)
         target [i] = tolower(source [i]);
      target[i] = '\0';
      }
   return target;
   }

char *strstrncase (char *haystack, char *needle) {
   char *workh, *workn, *found;
   if ((haystack == NULL) || (needle == NULL))
      return NULL;
   strtoupper (workn = (char *) malloc (strlen (needle)), needle);
   strtoupper (workh = (char *) malloc (strlen (haystack)), haystack);
   found = strstr (workh, workn);
   if (found != NULL)
      found = (found - workh) + haystack;
   free(workn);
   free(workh);
   return found;
   }

char *strip (char *target, char *source, char *oper) {
   int i, j;
   if (target == NULL)
      return NULL;
   if (source == NULL)
      target[0] = '\0';
   else {
      if (toupper(oper[0]) == 'B' || toupper(oper[0]) == 'L')
         for (i = 0; source[i] == ' '; i++);
      else
         i = 0;
      for (j = 0; source[i] != '\0'; j++, i++)
         target[j] = source[i];
      target[j] = '\0';
      if (toupper(oper[0]) == 'B' || toupper(oper[0]) == 'T') {
         for (i = j-1; i >= 0 && target[i] == ' '; i--);
         target [i+1] = '\0';
         }
      }
   return target;
   }

char *left (char *target, char *source, int len) {
   int i;
   if (target == NULL)
      return NULL;
   if (source == NULL)
      target[0] = '\0';
   else {
      for (i = 0; i < len || source[i] == '\0'; i++)
         target [i] = source [i];
      target[i] = '\0';
      }
   return target;
   }

char *right (char *target, char *source, int len) {
   DWORD i, tpos, slen;
   if (target == NULL)
      return NULL;
   if (source == NULL)
      target[0] = '\0';
   else {
      slen = strlen(source);
      for (i = max(0, slen - len), tpos = 0; i < slen; i++)
         target [tpos++] = source [i];
      target[tpos] = '\0';
      }
   return target;
   }

char *fillstrx(char *target, int len, char datachar) {
   int i;
   if (target == NULL)
      return NULL;
   for (i = strlen(target); i < len; i++)
         target[i] = datachar;
   target[len] = '\0';
   return target;
   }

char *fillstr(char *target, int len) {
   return fillstrx(target, len, ' ');
   }

char *strinit(char *target, int len, char datachar) {
   if (target == NULL)
      return NULL;
   target[0] = '\0';
   return fillstrx(target, len, datachar);
   }

char *substr (char *target, char *source, int pos, int len) {
   if (target == NULL)
      return NULL;
   if (source == NULL)
      target[0] = '\0';
   else {
      strncpy(target, &source[pos], len);
      fillstr(target, len);
      }
   return target;
   }

char *mid (char *target, char *source, int pos) {
   if (target == NULL)
      return NULL;
   if (source == NULL)
      target[0] = '\0';
   else
      strcpy(target, &source[pos]);
   return target;
   }

int pos(char *findstr, char *source) {
   char * pdest;
   if ((findstr == NULL) || (source == NULL))
      return 0;
   pdest = strstr(source, findstr);
   if (pdest == NULL)
      return 0;
   else
      return pdest + 1 - source;
   }

int lastpos(char *findstr, char *source) {
   char *ipos, *lastfoundpos;
   if ((findstr == NULL) || (source == NULL))
      return 0;
   ipos = source - 1;
   do {
      lastfoundpos = ipos;
      ipos = strstr(ipos + 1, findstr);
      } while (ipos != NULL);
   return lastfoundpos + 1 - source;
   }

char *parseword(char *target, char *source) {
   // this routine destroys/alters source
   DWORD ipos;
   if (target == NULL)
      return NULL;
   if (source == NULL)
      target[0] = '\0';
   else {
      strip(source, source, "L");
      ipos = pos(" ", source);
      if (ipos == 0) {
         strcpy(target, source);
         strcpy(source, "");
         }
      else {
         left(target, source, ipos - 1);
         mid(source, source, ipos);
         strip(source, source, "L");
         }
      }
   return target;
   }

char *justify (char * target, char * source, int tmax) {
   int slen;
   slen = strlen(source);
   if (slen >= tmax)
      right(target, source, tmax);
   else {
      strinit(target, tmax - slen, '0');
      strcat(target, source);
      }
   return target;
   }

char *justifynum (char *target, int source, long tmax) {
   char x[33];
   ltoa(source, x, 10);
   return justify(target, x, tmax);
   }

char * quoted(char * target, char * source) {
   sprintf(target, "'%s'", source);
   return target;
   }

#endif
