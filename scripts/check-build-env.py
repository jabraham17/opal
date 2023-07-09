import sys
import os
from typing import List


def dump_args(file: str, args: List[str]):
    with open(file, "w") as f:
        print(*args, sep="\n", file=f)


def main(args: List[str]) -> int:
    outfile = sys.argv[1]
    makeflags = sys.argv[2]
    args = [a.strip() for a in sys.argv[3:]]

    # if file doesn't exist, create it and return 0
    if not os.path.exists(outfile):
        dump_args(outfile, args)
        return 0

    # if makeflags has a B, force build so regen output
    if "-B" in makeflags:
        dump_args(outfile, args)
        return 0

    # file exists, check it
    lines = [l.strip() for l in open(outfile, "r").readlines()]

    if len(args) != len(lines):
        return 1

    for arg, line in zip(args, lines):
        if arg != line:
            return 1

    return 0


if __name__ == "__main__":
    exit(main(sys.argv[1:]))
