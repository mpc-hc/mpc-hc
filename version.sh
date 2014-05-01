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

versionfile_fixed="./include/version.h"
versionfile="./include/version_rev.h"
manifestfile="./src/mpc-hc/res/mpc-hc.exe.manifest"

# Read major, minor and patch version numbers from static version.h file
while read -r _ var value; do
  if [[ $var == MPC_VERSION_MAJOR ]]; then
    ver_fixed_major=$value
  elif [[ $var == MPC_VERSION_MINOR ]]; then
    ver_fixed_minor=$value
  elif [[ $var == MPC_VERSION_PATCH ]]; then
    ver_fixed_patch=$value
  fi
done < "$versionfile_fixed"
ver_fixed="${ver_fixed_major}.${ver_fixed_minor}.${ver_fixed_patch}"
echo "Version:   $ver_fixed"

# If we are not inside a git repo use hardcoded values
if ! git rev-parse --git-dir > /dev/null 2>&1; then
  hash=0000000
  ver=0
  ver_additional=
  echo "Warning: Git not available or not a git repo. Using dummy values for hash and version number."
else
  # Get information about the current version
  describe=$(git describe --long)
  echo "Describe:  $describe"

  # Get the abbreviated hash of the current changeset
  hash=${describe##*-g}

  # Get the number changesets since the last tag
  ver=${describe#*-}
  ver=${ver%-*}

  ver_additional=" ($hash)"

  # Get the current branch name
  branch=$(git symbolic-ref -q HEAD) && branch=${branch##refs/heads/} || branch="no branch"

  echo "On branch: $branch"
  echo "Hash:      $hash"
  if ! git diff-index --quiet HEAD; then
    echo "Revision:  $ver (Local modifications found)"
  else
    echo "Revision:  $ver"
  fi

  # If we are on another branch that isn't master, we want extra info like on
  # which commit from master it is based on. This assumes we
  # won't ever branch from a changeset from before the move to git
  if [[ "$branch" != "master" ]]; then
    version_info="#define MPCHC_BRANCH _T(\"$branch\")"$'\n'
    ver_additional+=" ($branch)"
    if git show-ref --verify --quiet refs/heads/master; then
      # Get where the branch is based on master
      base=$(git merge-base master HEAD)
      base=${base:0:7}
      ver_additional+=" (master@${base})"
      echo "Mergebase: master@${base}"
    fi
  fi
fi

version_info+="#define MPCHC_HASH _T(\"$hash\")"$'\n'
version_info+="#define MPC_VERSION_REV $ver"$'\n'
version_info+="#define MPC_VERSION_ADDITIONAL _T(\"${ver_additional}\")"


# Update version_rev.h if it does not exist, or if version information was changed.
if [[ ! -f "$versionfile" ]] || [[ "$version_info" != "$(<"$versionfile")" ]]; then
  # Write the version information to version_rev.h
  echo "$version_info" > "$versionfile"
fi


# Update manifest file if it does not exist or if source manifest.conf was changed.
newmanifest="$(sed -e "s/\\\$VERSION\\\$/${ver_fixed}.${ver}/" "$manifestfile.conf")"
if [[ ! -f "$manifestfile" ]] || [[ "$newmanifest" != "$(<"$manifestfile")" ]]; then
  # Update the revision number in the manifest file
  echo "$newmanifest" > "$manifestfile"
fi
