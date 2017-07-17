# To-do

## Short-term stuff

1. Migrate Trac to GitHub issues after we clean up any useless tickets
2. Should we migrate the closed tickets too?
3. Start fixing small bugs
4. Make DrDump optional - see if there are better alternatives
5. Improve nightly builds script to skip rebuilding LAV Filters if no relevant change has been committed
6. Investigate using AppVeyor for nightlies
7. Remove VirtualDub??
8. Drop DX SDK 11, probably along with XP drop

## MPC-HC

1. Remove support for unneeded/obsolete renderers
2. Drop XP support after its EOL

## MPCIconLib

1. Replace the file association icons with better looking ones

## WebServer

1. Update the images for player.html using CSS/SVG sprites
2. Add an alternative web interface if possible

## Installer

1. Add file association tasks, see <https://trac.mpc-hc.org/ticket/2207>

## VSFilter

1. Use the defines from version.h for `MPC_COPYRIGHT_STR` in VSFilter
2. Define VSFilter's version numbers in a place accessible by its installer and VSFilter itself
3. Backport a few useful patches from those VSFilter forks; we need individual patches for that
   and we have to make sure that everything works right for mpc-hc itself
4. Release the installer version too
