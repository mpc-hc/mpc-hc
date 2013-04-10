# Contributing to MPC-HC

## Reporting Issues

1. Make sure you have access to our [Trac](https://trac.mpc-hc.org/login) (note that
   [OpenID authentification](https://trac.mpc-hc.org/openidlogin) is available).
2. Please search our [Trac](https://trac.mpc-hc.org/report/1)
   for your problem since there's a good chance that someone has already reported it.
3. In case you found a match, please try to provide as much info as you can
   so we have better picture about what the real problem is and how to fix it ASAP.
4. If you didn't find any tickets with a problem similar to yours then please open a
   [new ticket](https://trac.mpc-hc.org/ticket/newticket)
   * Be descriptive as much as you can
   * Provide screenshots, samples, system/hardware information

## Pull Requests

If you want to help, here's what you need to do:

1. Make sure you have a [GitHub account](https://github.com/signup/free).
2. [Fork](https://github.com/mpc-hc/mpc-hc/fork_select) the repository you wish to help on.
3. Create a new topic branch to contain your feature, change, or fix.
4. Set `core.autocrlf` to true: `git config core.autocrlf true`.
5. Make sure you have enabled the precommit hook **[checkyear.sh](/contrib/checkyear.sh)**.
6. Make sure that your changes adhere to the current coding conventions used
   throughout the project - indentation, accurate comments, etc.
   For the style part we use [AStyle](http://astyle.sourceforge.net/),
   so please **run `contrib/run_astyle.bat`** before you push your changes.
7. [Open a Pull Request](https://github.com/mpc-hc/mpc-hc/pulls) with a clear title and description.
