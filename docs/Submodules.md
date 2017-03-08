# Updating submodules

## Prerequisites

Add a new remote to LAV Filters and FFmpeg submodules:

* for LAV Filters, `git remote add upstream https://github.com/Nevcairiel/LAVFilters.git` in **src/thirdparty/LAVFilters/src**
* for FFmpeg, `git remote add upstream git://git.1f0.de/ffmpeg.git` in **src/thirdparty/LAVFilters/src/ffmpeg**

**Warning:** Before updating LAV Filters always makes sure that no update is required on MPC-HC side.
If there are some changes in LAV Filters interfaces or settings, some changes are likely
to be needed in MPC-HC. If the update breaks compatibility with older LAV Filters versions,
remember to update the version check in **FGFilterLAV.cpp**.

## How to update LAV Filters

1. Checkout the master branch in FFmpeg submodule (**src/thirdparty/LAVFilters/src/ffmpeg**)
2. Do `git remote update` to update remotes
3. Do `git reset origin/master --hard` to clean up local repository (beware that you will lose all local commits)
4. Do `git rebase upstream/master` to update FFmpeg
5. Apply new custom patches, if any
6. Do `git tag mpc-hc-X.Y.Z-N` where X.Y.Z is the latest MPC-HC version
   and N is the number of LAV Filters updates since that release
7. Do `git push --force --tags origin master` to update our FFmpeg repository
8. Checkout the master branch in LAV Filters submodule (src/thirdparty/LAVFilters/src)
9. Do `git remote update` to update remotes
10. Do `git reset origin/master --hard` to clean up local repository (beware you will lose all local commits)
11. Do `git rebase upstream/master` to update LAV Filters
12. Apply new custom patches if any
13. Do `git tag mpc-hc-X.Y.Z-N` where X.Y.Z is the latest MPC-HC version
    and N is the number of LAV Filters updates since that release
14. Do `git push --force --tags origin master` to update our LAV Filters repository
15. Commit the submodule update in MPC-HC repository
