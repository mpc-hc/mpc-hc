# Contributing to MPC-HC

## Pull Requests

If you want to help, here's what you need to do:

1. Make sure you have a [GitHub account](https://github.com/signup/free).
2. [Fork](https://github.com/mpc-hc/mpc-hc/fork) our repository.
3. Create a new topic branch (based on the `develop` branch) to contain your feature, change, or fix.
4. **Set `core.autocrlf` to true: `git config core.autocrlf true`.**
5. **Make sure you have enabled the pre-commit hook - [pre-commit.sh](/contrib/pre-commit.sh)**.
   Open the file and read the comments for more info.
6. Make sure that your changes adhere to the current coding conventions used
   throughout the project - indentation, accurate comments, etc.
   For the style part we use [AStyle](http://astyle.sourceforge.net/),
   so please **run `contrib/run_astyle.bat` before you push your changes**. Get the binary
   from their site and put it somewhere in your `PATH`.
7. [Open a Pull Request](https://github.com/mpc-hc/mpc-hc/pulls) with a clear title and description.

### General development guidelines

1. Apart from the above instructions, try to keep your patches clean **without** any unrelated changes.
2. Keep your branches in good shape; don't mix patches that do different things and **always**
   try to squash when it makes sense. For example, you made 3 patches that refer to the same thing;
   squash them into one.
3. Since nightlies don't run on CI (yet), never push trivial patches to the develop branch; we don't
   want to trigger a nightly just for cosmetic or trivial patches. That being said, we try to push
   in batches. The nightlies's script runs every night at 01:00 GMT+2 (see <https://nightly.mpc-hc.org/>)
4. Never merge something that hasn't been reviewed by someone other on the team. This is so that we
   keep the risk of breaking stuff to the minimum since we don't have any tests.
5. When the resource files change, make sure you run [sync.bat](/src/mpc-hc/mpcresources/sync.bat)
   (requires Python so check [Compilation.md](/docs/Compilation.md)'s Part B) to sync the resource files
   and verify that everything went right.
6. Generally we try not to touch third-party code. If some change is needed there:
   1. Try to provide patches to the upstream project.
   2. If for any reason this takes a long time or isn't possible, make a clear patch, mark custom code as
      such.
7. Try to make sure all newly added files have a license header with copyright info and the year it was
   created/edited.

## Reporting Issues

1. Make sure you have access to our [Trac](https://trac.mpc-hc.org/login).
2. Please search our [Trac](https://trac.mpc-hc.org/report/1) for your problem since there's a good
   chance that someone has already reported it.
3. If you find a match, please try to provide as much info as you can,
   so that we have a better picture about what the real problem is and how to fix it ASAP.
4. If you didn't find any tickets with a problem similar to yours then please open a
   [new ticket](https://trac.mpc-hc.org/ticket/newticket)
   * Be descriptive as much as you can
   * Provide screenshots, samples, system/hardware information
