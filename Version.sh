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
SVNREV=5588
SVNHASH="2f7d963f8be3f6b4c0b3c1916baf4408850055c7"
# Get the current branch name
BRANCH=`git branch | grep "^\*" | awk '{print $2}'`
# If we are on the master branch
if [ $BRANCH = "master" ] ; then
    # Count how many changesets we have since the last svn changeset
    VER=`git rev-list $SVNHASH..HEAD | wc -l`
    # Now add it with the last svn revision number
    VER=$(($VER+$SVNREV))
    # Because we don't want the branch name or the hash for master, we'll need
    # to check if MPCHC_BRANCH and MPCHC_HASH are defined in Version.h before
    # writing them to the file description
    echo "#define MPC_VERSION_REV $VER" > ./include/Version_rev.h
# If we are on another branch that isn't master, we want extra info like on
# which commit from master it is based on and what is its hash. This assumes we
# won't ever branch from a changeset from before the move to git
else
    # Get where the branch is based on master
    BASE=`git merge-base master HEAD`
    # Get the abbreviated hash of the current changeset
    HASH=`git log -n1 --format=%h`
    # Count how many changesets we have since the last svn changeset
    VER=`git rev-list $SVNHASH..$BASE | wc -l`
    # Now add it with to last svn revision number
    VER=$(($VER+$SVNREV))

    # write variables to Version_rev.h
    echo "#define MPCHC_BRANCH $BRANCH"> ./include/Version_rev.h
    echo "#define MPC_VERSION_REV $VER">> ./include/Version_rev.h
    echo "#define MPCHC_HASH $HASH">> ./include/Version_rev.h
fi
