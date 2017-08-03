#!/bin/bash
# (C) 2013-2015, 2017 see Authors.txt
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

# A pre-commit hook that allows to check if all changed and added files
# have the current year as copyright information and affirm
# that they have been astyled properly.

# To enable the hook, copy this file to .git/hooks/pre-commit (no extension)
# or run this while in the repo root:
# cp contrib/pre-commit.sh .git/hooks/pre-commit

# Astyle functionality is disabled by default, if you want to use it
# you should edit .git/hooks/pre-commit file and change
# astyle_enabled=n to astyle_enabled=y

# here you can enable/disable features
checkyear_enabled=y
astyle_enabled=n
astyle_ignore_excluded=y
astyle_ignore_stashed=n

# internal variables
versioncheck_version=6
versioncheck_path=contrib/pre-commit.sh
astyle_config=contrib/astyle.ini
astyle_extensions=(cpp h)
astyle_version='Artistic Style Version 3.0.1'
checkyear_extensions=(bat cpp h hlsl iss po py sh)
checkyear_pattern1='\(C\) (([0-9][0-9][0-9][0-9]-)?[0-9][0-9][0-9][0-9](, )?)+ see Authors.txt'
year=$(date +%Y)
checkyear_pattern2=''"$year"' see Authors.txt'

if [[ "$OSTYPE" == 'cygwin' ]]; then
  set -o igncr
fi

# print version and exit if requested
if [[ "$@" == '--version' ]]; then
  echo "$versioncheck_version"
  exit
fi

# warn when the tree has newer version of this script
if (( $((`bash "$versioncheck_path" --version` + 0)) > $versioncheck_version )); then
  echo "Warning: .git/hooks/pre-commit is older than $versioncheck_path, you should upgrade"
fi

in_list() {
  one="$1"
  shift
  for valid in "$@"; do
    [[ "$one" == "$valid" ]] && return 0
  done
  return 1
}

not_in_list() {
  in_list "$@" && return 1
  return 0
}

filepath_contains() {
  IFS='/' read -a parsed_path <<< "$1"
  shift
  for one in "${parsed_path[@]}"; do
    for valid in "$@"; do
      [[ "$one" == "$valid" ]] && return 0
    done
  done
  return 1
}

check_copyright_year() {
  return_code=0
  for file in "$@"; do
    result=$(grep -E "$checkyear_pattern1" "$file");
    if [[ -n $result ]]; then
      # check if the year is outdated
      if [[ $result != *$checkyear_pattern2 ]]; then
        echo "Invalid copyright year in $file"
        return_code=1
      fi
    fi
  done
  return $return_code
}

apply_astyle() {
  return_code=0
  if (( $# > 0 )); then
    astyle "--options=$astyle_config" "$@"
    return_code=$?
  fi
  return $return_code
}

# populate the list of stashed files
# if the feature is disabled, leave it empty
stashed_files=()
if [[ "$astyle_enabled" == y ]] && [[ "$astyle_ignore_stashed" == y ]]; then
  buffer=$(git stash list --format=%gd)
  [[ -n "$buffer" ]] &&
  while read -r stash; do
    stash_buffer=$(git stash show "${stash}" --name-only --diff-filter=M)
    [[ -n "$stash_buffer" ]] &&
    while read -r file; do
      if not_in_list "$file" "${stashed_files[@]}"; then
        stashed_files=("${stashed_files[@]}" "$file")
      fi
    done <<< "$stash_buffer" # process substitution is not implemented in msys bash
  done <<< "$buffer" # process substitution is not implemented in msys bash
fi

# parse astyle config file for excluded files and populate the list of such files
# if the feature is disabled, leave the list empty
astyle_excluded=()
if [[ "$astyle_enabled" == y ]] && [[ "$astyle_ignore_excluded" == y ]]; then
  buffer=$(grep -v ^# "$astyle_config")
  for pair in $buffer; do
    switch="${pair%%=*}"
    value="${pair##*=}"
    [[ "$switch" == "--exclude" ]] && astyle_excluded=("${astyle_excluded[@]}" "$value")
  done
fi

# loop through all new and modified files
checkyear_files=()
astyle_files=()
buffer=$(git diff --cached --name-only --diff-filter=ACMR)
[[ -n "$buffer" ]] &&
while read -r file; do
  ext="${file##*.}"

  # check whether the file may have mpc-hc copyright header
  if in_list "$ext" "${checkyear_extensions[@]}"; then
    checkyear_files=("${checkyear_files[@]}" "$file")
  fi

  # check whether the file is astylable and not present in git stashes
  # the latter might result in 'git stash pop' conflicts
  if [[ "$astyle_enabled" == y ]] && in_list "$ext" "${astyle_extensions[@]}"; then
    if ! filepath_contains "$file" "${astyle_excluded[@]}"; then
      if not_in_list "$file" "${stashed_files[@]}"; then
        astyle_files=("${astyle_files[@]}" "$file")
      else
        echo "Not astyling stashed $file"
      fi
    else
      echo "Not astyling blacklisted $file"
    fi
  fi
done <<< "$buffer" # process substitution is not implemented in msys bash

# do the actual work here
exit_code=0
if [[ "$checkyear_enabled" == y ]] && (( ${#checkyear_files[@]} > 0 )); then
  check_copyright_year "${checkyear_files[@]}" || exit_code=1
fi
if [[ "$astyle_enabled" == y ]] && (( ${#astyle_files[@]} > 0 )); then
  astyle_found_version=$(astyle --version 2>&1)
  if (( $? == 0 )); then
    if [[ "$astyle_found_version" == "$astyle_version" ]]; then
      apply_astyle "${astyle_files[@]}" && git add "${astyle_files[@]}" || exit_code=1
    else
      echo "Error: astyle version must be '$astyle_version'"
      exit_code=1
    fi
  else
    echo "Error: astyle was not found"
    exit_code=1
  fi
fi

# if there were problems, exit nonzero
if (( exit_code != 0 )); then
  echo "To ignore this and commit anyway use 'git commit --no-verify'"
fi
exit $exit_code
