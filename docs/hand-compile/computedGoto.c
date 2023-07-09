

long match_aaStar(char* input, long n) {
  long i = 0;
  long longestMatch = -1;
  char next = '\0';
aaStar_start : {
  if(i >= n) goto aaStar_done;
  next = input[i++];
  if(next == '\0') goto aaStar_done;
  if(next == 'a') goto aaStar_accept;
  else goto aaStar_done;
}
aaStar_accept : {
  longestMatch = i;
  if(i >= n || input[i] == '\0') goto aaStar_done;
  next = input[i++];
  if(next == 'a') goto aaStar_accept;
  else goto aaStar_done;
}

aaStar_done:
  return longestMatch;
}

long match_borcd(char* input, long n) {

  long i = 0;
  long longestMatch = -1;
  char next = '\0';

borcd_start : {
  if(i >= n || input[i] == '\0') goto borcd_done;
  next = input[i++];
  if(next == 'b') goto borcd_state2;
  else if(next == 'c') goto borcd_state2;
  else goto borcd_done;
}
borcd_state2 : {
  if(i >= n || input[i] == '\0') goto borcd_done;
  next = input[i++];
  if(next == 'd') goto borcd_accept;
  else goto borcd_done;
}
borcd_accept : {
  longestMatch = i;
  goto borcd_done;
}

borcd_done:
  return longestMatch;
}
