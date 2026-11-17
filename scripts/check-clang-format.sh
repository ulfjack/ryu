#!/usr/bin/env bash
# Verifies that all ryu C/C++ sources are clang-format clean.
#
# Big generated tables are protected by `// clang-format off/on` markers in
# the files themselves (digit_table.h, d2s_small_table.h, *_full_table.h,
# generic_128.h, tests/d2fixed_test.cc), so every source file under ryu/ is
# checked. .c/.h/.cc/.cpp/.hh are all covered.
#
# Requires clang-format 18.x (CI installs clang-format==18.1.8 via pip).
set -euo pipefail
cd "$(dirname "$0")/.."

mapfile -t FILES < <(git ls-files | grep -E '^ryu/.*\.(c|h|cc|cpp|hh)$')

echo "${#FILES[@]} files"
clang-format --dry-run --Werror "${FILES[@]}"
