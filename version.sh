#!/bin/bash
# (C) 2012-2013 see Authors.txt
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

# This is the last svn changeset, the number and hash can be automatically
# calculated, but it is slow to do that. So it is better to have it hardcoded.
svnrev=5597
svnhash="f669833b77e6515dc5f0a682c5bf665f9a81b2ec"

versionfile="./include/version_rev.h"
manifestfile="./src/mpc-hc/res/mpc-hc.exe.manifest"

#If the git command isn't available or we are not inside a git repo use hardcoded values
if ! git rev-parse --git-dir > /dev/null 2>&1; then
  hash=0000000
  ver=0
else
  # Get the current branch name
  branch=$(git symbolic-ref -q HEAD) && branch=${branch##refs/heads/} || branch="no branch"
  # If we are on the master branch
  if [[ "$branch" == "master" ]]; then
    base="HEAD"
  # If we are on another branch that isn't master, we want extra info like on
  # which commit from master it is based on and what its hash is. This assumes we
  # won't ever branch from a changeset from before the move to git
  else
    # Get where the branch is based on master
    base=$(git merge-base master HEAD)
    base_ver=$(git rev-list --count $svnhash..$base)
    base_ver=$((base_ver+svnrev))

    version_info="#define MPCHC_BRANCH _T(\"$branch\")"$'\n'
    ver_full=" ($branch) (master@${base_ver:0:7})"
  fi

  # Count how many changesets we have since the last svn changeset
  ver=$(git rev-list --count $svnhash..HEAD)
  # Now add it with to last svn revision number
  ver=$((ver+svnrev))

  # Get the abbreviated hash of the current changeset
  hash=$(git rev-parse --short HEAD)

fi

ver_full="_T(\"$ver ($hash)$ver_full\")"

version_info+="#define MPCHC_HASH _T(\"$hash\")"$'\n'
version_info+="#define MPC_VERSION_REV $ver"$'\n'
version_info+="#define MPC_VERSION_REV_FULL $ver_full"

if [[ "$branch" ]]; then
  echo "On branch: $branch"
fi
echo "Hash:      $hash"
if [[ "$branch" ]] && ! git diff-index --quiet HEAD; then
  echo "Revision:  $ver (Local modifications found)"
else
  echo "Revision:  $ver"
fi
if [[ "$branch" ]] && [[ "$branch" != "master" ]]; then
  echo "Mergebase: master@${base_ver} (${base:0:7})"
fi

# Update version_rev.h if it does not exist, or if version information.
if [[ ! -f "$versionfile" ]] || [[ "$version_info" != "$(<"$versionfile")" ]]; then
  # Write the version information to version_rev.h
  echo "$version_info" > "$versionfile"
fi

# Update manifest file if it does not exist, if version information changed,
# or if source manifest.conf was changed.
newmanifest="$(sed -e "s/\\\$WCREV\\\$/${ver}/" "$manifestfile.conf")"
if [[ ! -f "$manifestfile" ]] || [[ "$newmanifest" != "$(<"$manifestfile")" ]]; then
  # Update the revision number in the manifest file
  echo "$newmanifest" > "$manifestfile"
fi
