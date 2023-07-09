

// dfa for aa*
// returns number of chars matched, or -1 for no match
long match_aaStar(char* input, long n) {
  // three states
  const int STATE_start = 0;
  const int STATE_null = 1;
  const int STATE_accept = 2;
  int state = STATE_start;
  long i = 0;
  long longestMatch = -1;
  int done = 0;
  while(!done && i < n && input[i] != '\0') {
    char next = input[i];
    switch(state) {
      case STATE_start: {
        if(next == 'a') state = STATE_accept;
        else state = STATE_null;
      } break;
      case STATE_accept: {
        longestMatch = i;
        if(next == 'a') state = STATE_accept;
        else state = STATE_null;
      } break;
      case STATE_null: {
        state = STATE_null;
        //  can set done because we never escape this state
        done = 1;
      } break;
    }
    i++;
  }
  if(state == STATE_accept) longestMatch = i;
  return longestMatch;
}

// dfa for (b|c)d
long match_borcd(char* input, long n) {
  // three states
  const int STATE_start = 0;
  const int STATE_null = 1;
  const int STATE_inter = 2;
  const int STATE_accept = 3;
  int state = STATE_start;
  long i = 0;
  long longestMatch = -1;
  int done = 0;
  while(!done && i < n && input[i] != '\0') {
    char next = input[i];
    switch(state) {
      case STATE_start: {
        if(next == 'b') state = STATE_inter;
        else if(next == 'c') state = STATE_inter;
        else state = STATE_null;
      } break;
      case STATE_inter: {
        if(next == 'd') state = STATE_accept;
        else state = STATE_null;
      } break;
      case STATE_accept: {
        longestMatch = i;
        state = STATE_null;
      } break;
      case STATE_null: {
        state = STATE_null;
        //  can set done because we never escape this state
        done = 1;
      } break;
    }
    i++;
  }
  if(state == STATE_accept) longestMatch = i;
  return longestMatch;
}
