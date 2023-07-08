
enum aaStar_states {
  aaStar_state_start,
  aaStar_state_null,
  aaStar_state_accept
};

static inline void handleState_aaStar_start(
    char c,
    int i,
    int* longestMatch,
    enum aaStar_states* state,
    int* done) {
  if(c == 'a') *state = aaStar_state_accept;
  else *state = aaStar_state_null;
}
static inline void handleState_aaStar_accept(
    char c,
    int i,
    int* longestMatch,
    enum aaStar_states* state,
    int* done) {
  *longestMatch = i;
  if(c == 'a') *state = aaStar_state_accept;
  else *state = aaStar_state_null;
}
static inline void handleState_aaStar_null(
    char c,
    int i,
    int* longestMatch,
    enum aaStar_states* state,
    int* done) {
  *state = aaStar_state_null;
  //  can set done because we never escape this state
  *done = 1;
}

// dfa for aa*
// returns number of chars matched, or -1 for no match
int match_aaStar(char* input, int n) {

  // three states
  enum aaStar_states state = aaStar_state_start;
  int i = 0;
  int longestMatch = -1;
  int done = 0;
  while(!done && i < n && input[i] != '\0') {
    char next = input[i];
    switch(state) {
      case aaStar_state_start:
        handleState_aaStar_start(next, i, &longestMatch, &state, &done);
        break;
      case aaStar_state_accept:
        handleState_aaStar_accept(next, i, &longestMatch, &state, &done);
        break;
      case aaStar_state_null:
        handleState_aaStar_null(next, i, &longestMatch, &state, &done);
        break;
    }
    i++;
  }
  if(state == aaStar_state_accept) longestMatch = i;
  return longestMatch;
}

int match_borcd(char* input, int n) { return -1; }
