#ifndef __TOKEN_H__
#define __TOKEN_H__

#ifdef __cplusplus
  extern "C" {
#endif

struct STR_SAVED_TOKENS_ {
        char *str_next_saved_tokens;
        char str_ch_saved_tokens;
};

typedef struct STR_SAVED_TOKENS_ STR_SAVED_TOKENS;

char *StrTokenize(char *str, const char * const token);
void StrTokSave(STR_SAVED_TOKENS *st);
void StrTokRestore(STR_SAVED_TOKENS *st);

#define StrTokStop() (void)StrTokenize(0, 0)

#ifdef __cplusplus
  }
#endif

#endif
