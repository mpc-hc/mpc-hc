#!/bin/bash
# (C) 2013 see Authors.txt
#
# This file is part of MPC-HC.
#
# MPC-HC is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# MPC-HC is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.


# pre-commit hook to check if all changed and added files
# have the current year as copyright information.

# to enable the hook, copy this file to .git/hooks/pre-commit (no extension)
# or run this while in the repo root:
# cp contrib/checkyear.sh .git/hooks/pre-commit

year=$(date +%Y)
pattern1='\(C\) ([0-9][0-9][0-9][0-9]-)?[0-9][0-9][0-9][0-9] see Authors.txt'
pattern2='\(C\) ([0-9][0-9][0-9][0-9]-)?'"$year"' see Authors.txt'
extensions=(bat cpp h iss pl)

valid_extension() {
  for e in "${extensions[@]}"; do
    [[ "$e" == "$1" ]] && return 0
  done
  return 1
}

# save output to variable, because MSYS bash doesn't support command substitution
list=$(git diff --cached --name-only --diff-filter=ACMR)

# loop through all new and modified files
error=0
while read -r f; do
  # check if the file has an extension that we care about
  if valid_extension "${f##*.}"; then
    # only verify year if it already has an mpc-hc style copyright entry
    if grep -q -E "$pattern1" "$f"; then
      # check if the year is outdated
      if ! grep -q -E "$pattern2" "$f"; then
        # print filename
        echo "$f"
        # make sure we exit nonzero, later
        error=1
      fi
    fi
  fi
done <<< "$list"

# if there were files with outdated copyright, exit nonzero
if (( error == 1 )); then
  echo "Copyright year in the file(s) listed above is outdated!"
  echo "To ignore this and commit anyways use 'git commit --no-verify'"
  exit 1
fi

# all files valid
exit 0
