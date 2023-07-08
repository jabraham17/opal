

typedef long (*matchFunc)(char*, long);
long match_aaStar(char* input, long n);
long match_borcd(char* input, long n);
long match_bacStard(char* input, long n);
long match(char*, long);

matchFunc funcs[] = {match_aaStar, match_borcd, match_bacStard, match};
char* regexNames[] = {"aa*", "(b|c)d", "(ba|c*)d", "ab"};
int n_funcs = 4;
