
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "match_funcs.h"

int main(int argc, char** argv) {

  for(int i = 1; i < argc; i++) {

    char* word = argv[i];
    long len = strlen(word);

    long match = -1;
    int funcNum;
    for(funcNum = 0; funcNum < n_funcs; funcNum++) {
      matchFunc f = funcs[funcNum];
      match = f(word, len);
      if(match >= 0 && match == len) break;
    }
    if(match >= 0 && match == len) {
      printf("'%s' matched '%s'\n", word, regexNames[funcNum]);
    } else {
      printf("'%s' did not match anything\n", word);
    }
  }

  return 0;
}
