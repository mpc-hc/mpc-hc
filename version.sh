#!/bin/sh
# (C) 2012 see Authors.txt
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

[ -n "$1" ] && cd $1
# This is the last svn changeset, the number and hash can be automatically
# calculated, but it is slow to do that. So it is better to have it hardcoded.
# We'll need to update this with the last svn data before committing this script
SVNREV=5597
SVNHASH="f669833b77e6515dc5f0a682c5bf665f9a81b2ec"

if [ ! -d ".git" ] ; then
  # If the folder ".git" doesn't exist we use hardcoded values
  HASH=0000000
  VER=0
else
  # Get the current branch name
  BRANCH=`git branch | grep "^\*" | awk '{print $2}'`
  # If we are on the master branch
  if [ "$BRANCH" == "master" ] ; then
    BASE="HEAD"
  # If we are on another branch that isn't master, we want extra info like on
  # which commit from master it is based on and what its hash is. This assumes we
  # won't ever branch from a changeset from before the move to git
  else
    # Get where the branch is based on master
    BASE=`git merge-base master HEAD`

    VERSION_INFO+="#define MPCHC_BRANCH _T(\"$BRANCH\")\n"
  fi

  # Count how many changesets we have since the last svn changeset
  VER=`git rev-list $SVNHASH..$BASE | wc -l`
  # Now add it with to last svn revision number
  VER=$(($VER+$SVNREV))

  # Get the abbreviated hash of the current changeset
  HASH=`git log -n1 --format=%h`

fi

VERSION_INFO+="#define MPCHC_HASH _T(\"$HASH\")\n"
VERSION_INFO+="#define MPC_VERSION_REV $VER"

if [ "$BRANCH" ] ; then
  echo -e "On branch: $BRANCH"
fi
echo -e "Hash:      $HASH"
if [ "$BRANCH" ] && git status | grep -q "modified:" ; then
  echo -e "Revision:  $VER (Local modifications found)"
else
  echo -e "Revision:  $VER"
fi

if [ -f ./include/version_rev.h ] ; then
  VERSION_INFO_OLD=`<./include/version_rev.h`
fi

# Only write the files if the version information has changed
if [ "$(echo $VERSION_INFO | sed -e 's/\\n/ /g')" != "$(echo $VERSION_INFO_OLD)" ] ; then
  # Write the version information to version_rev.h
  echo -e $VERSION_INFO > ./include/version_rev.h

  # Update the revision number in the manifest file
  sed -e "s/\\\$WCREV\\\$/${VER}/" ./src/mpc-hc/res/mpc-hc.exe.manifest.conf > ./src/mpc-hc/res/mpc-hc.exe.manifest
fi
