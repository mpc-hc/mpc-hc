# Release procedure for stable builds

We use a Git workflow based on <http://nvie.com/posts/a-successful-git-branching-model/>.

The main idea is that the development takes place in a branch named `develop` instead of `master`.
All pull requests and more generally all "feature" branches must originate from `develop` and be
merged back into it. When a release is planned, a new temporary branch `release-X.Y.Z` is created
from `develop` and finally merged into `master` when the release is ready to be published. The only
difference with the workflow proposed on nvie.com is that we merge `master` into `develop` after
`release-X.Y.Z` has been merged instead of directly merging `release-X.Y.Z` into `develop`. This
ensures the release tags are visible from the `develop` branch to have a proper numbering of the
nightly builds. In case a hotfix is needed, a new temporary `hotfix-X.Y.Z` branch is created from
the head of the `master` branch and merged back into `master`. `master` is then merged into `develop`.

Here is a quick how-to release a new stable build:

1. Create a new branch `release-X.Y.Z` from the commit on `develop` branch you want to use as a base
   for the next stable release (you don't have to always use the latest commit from `develop`)
2. Do everything you want to prepare the release on this newly created branch. New commits can still
   be added on the `develop` branch if they aren't to be included in the release.
3. When you are ready to release, make sure to:

    * Update version in **include/version.h**
    * Update **thirdparty/versions.txt** and the version/release date in **Changelog.txt**
    * Commit those changes

4. Merge the `release-X.Y.Z` branch into `master`. Use `git merge --no-ff release-X.Y.Z` to make sure
   the branch history is correctly saved. The temporary branch `release-X.Y.Z` can now be removed using
   `git branch -d release-X.Y.Z`
5. Tag the merge commit for the new release: `git tag -a X.Y.Z -m "Tag vX.Y.Z"`
6. Merge back the `master` branch into `develop`. Use `git merge --no-ff --no-commit master` since it's
   likely that some manually editing of the merge commit is needed to merge the changelog correctly. Try
   to give the merge commit an informative commit log, mentioning the release and its version number
7. Checkout the `master` branch and make sure you have a clean source tree, no modified files, or unpushed commits
8. Compile MPC-HC and the standalone filters:

   1. `CALL "build.bat" Clean All Both Release`
   2. `CALL "build.bat" Build All Both Release Packages`

9. Keep the PDB files of all the filters and MPC-HC builds
10. Upload the PDB files and the corresponding binary files to DrDump server. Use the same command
    as the nightly builds with the version set to `X.Y.Z.0`
11. Upload the binary packages on MaxCDN following the directory and the packages names scheme
    (upload the PDB files too, use 7-zip to create the 7z packages)
12. Upload the old stable release on <stable.mpc-hc.org>
13. Update the website with the new version number, changelog and download links
14. Update the milestone and the changelog on Trac
