# regex

```text
# comments can appear anywhere
# groups are (?:...), captures are ()

# regex for a.b+
START CAPTURE 0
MATCH 1 CHAR A
MATCH 1 WILD
MATCH + CHAR B
END CAPTURE 0
```

```text
# my dfa definition
s0->s1;char1,char2,char3
# nfa is same, with epsilon

# regex, dots used to simplify parsing
(a.b.c*)|a
```

implement efficent regex by compiling to my simple regex, then nfa, then dfa, then pruning dfa and building a FSM in C
We can handle wild cards with special states in the DFA
