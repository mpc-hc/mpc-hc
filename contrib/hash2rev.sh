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


[[ -n "$1" ]] || exit 1
# This is the last svn changeset, the number and hash can be automatically
# calculated, but it is slow to do that. So it is better to have it hardcoded.
svnrev=5597
svnhash="f669833b77e6515dc5f0a682c5bf665f9a81b2ec"

# If the git command isn't available or we are not inside a git repo exit the script
git rev-parse --git-dir > /dev/null 2>&1 || exit 1

# If the input hash is after the switch to git
if [[ $(git merge-base $svnhash "$1") == $(git rev-parse --verify "$svnhash") ]] ; then
  # Count how many changesets we have since the last svn changeset until the input hash
  ver=$(git rev-list --count "$svnhash..$1")
  # Now add it with to last svn revision number
  ver=$((ver+svnrev))
else
  # Get the revision by parsing the git-svn-id
  ver=$(git log -1 --format="%b" "$1")
  ver=${ver#*@}
  ver=${ver%% *}
fi

echo "$ver"
