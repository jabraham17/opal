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


feature
- powerset construction
- fix parser
  - allow basic concat, union, kleene, and grouping
  - add `+`, `?`
  - add `()` as capturing groups
  - add `(?:)`, supports non capturing groups
  - add `[]`
  - add `.`
  - non greedy matching `+?` and `*?`
  - add `[^]`
  - add `{n}`
  - add `{min,}`
  - add `{,max}`
  - add `{min,max}`
  - add `^` and `$`, supports search vs match
  - back references, maybe doing a limited max of 9 or soemething
  - possessive matching
  - look ahead and look behind
  - consider predefined character groups
- add support to extract groups
- add a search vs match mode
- jitted regex


software engineering todo
- add my suite of makefile and scripts
- separate out files
- ensure better const correctness
- toC, toNasm, etc using visiotor pattern
- add iterator support for states and transitions
