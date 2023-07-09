#!/usr/bin/env python3
import os
import argparse as ap
from typing import List


def normpath(path: str, root: str):
    local_path = os.path.relpath(path, os.path.commonpath([root, path]))
    local_path = os.path.normpath(local_path)
    return local_path


def allHeaders(path: str, skips: List[str]):
    for dirpath, dirnames, filenames in os.walk(path):
        for filename in filenames:
            p = os.path.join(dirpath, filename)
            norm_path = normpath(p, path)
            if norm_path in skips:
                continue
            if os.path.splitext(p)[1] in [".h", ".hpp"]:
                yield p


def filterName(name: str):
    name = name.replace("-", "_")
    name = name.upper()
    return name


def main():
    args = ap.ArgumentParser()
    args.add_argument("--fix", action="store_true")
    args.add_argument("--skip", action="append", type=str, default=[])
    args.add_argument("--prefix", type=str, default="ONCE_")
    args.add_argument("--suffix", type=str, default="_H_")
    args.add_argument("--root", type=str, default=".")

    args = args.parse_args()

    fix = args.fix
    ifdef_prefix = args.prefix
    ifdef_suffix = args.suffix
    src_root = os.path.abspath(args.root)
    skips = args.skip

    return_code = 0

    for h in allHeaders(src_root, skips):
        local_path = normpath(h, src_root)
        local_dir = os.path.dirname(local_path)
        parts = local_dir.split(os.sep) + [os.path.splitext(os.path.basename(h))[0]]
        ifdef_name = ifdef_prefix + "_".join(parts) + ifdef_suffix
        ifdef_name = filterName(ifdef_name)
        lines = open(h, "r").readlines()
        if len(lines) < 2:
            print(f"'{local_path}' may be missing an include guard")
            return_code = 1
        else:
            ifdef = lines[0].strip()
            define = lines[1].strip()
            if ifdef == f"#ifndef {ifdef_name}" and define == f"#define {ifdef_name}":
                # this is fine
                pass
            elif ifdef.startswith("// #ifndef"):
                pass
            elif not ifdef.startswith("#ifndef"):
                print(f"'{local_path}' missing include guard")
                if fix:
                    with open(h, "w") as f:
                        f.write(f"#ifndef {ifdef_name}\n")
                        f.write(f"#define {ifdef_name}\n")
                        l = "".join(lines)
                        f.write(l)
                        f.write("#endif\n")
                else:
                    return_code = 1
            else:
                print(f"'{local_path}' has a bad include guard")
                if fix:
                    with open(h, "w") as f:
                        lines[0] = f"#ifndef {ifdef_name}\n"
                        lines[1] = f"#define {ifdef_name}\n"
                        l = "".join(lines)
                        f.write(l)
                else:
                    return_code = 1

    return return_code


if __name__ == "__main__":
    exit(main())
