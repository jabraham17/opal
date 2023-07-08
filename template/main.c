#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef long (*matchFunc)(const char*, long);
extern long nPatterns;
extern matchFunc funcs[];
extern const char* patterns[];

int main(int argc, const char** argv) {

  for(int i = 1; i < argc; i++) {

    const char* word = argv[i];
    long len = strlen(word);

    long match = -1;
    long funcNum;
    for(funcNum = 0; funcNum < nPatterns; funcNum++) {
      matchFunc f = funcs[funcNum];
      match = f(word, len);
      if(match >= 0 && match == len) break;
    }
    if(match >= 0 && match == len) {
      printf("'%s' matched '%s'\n", word, patterns[funcNum]);
    } else {
      printf("'%s' did not match anything\n", word);
    }
  }

  return 0;
}
