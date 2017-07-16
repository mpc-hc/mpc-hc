# To-do

Patches are always welcome. :)

## MPC-HC

1. Remove support for unneeded/obsolete renderers
2. Drop XP support after its EOL

## MPCIconLib

1. Replace the file association icons with better looking ones

## WebServer

1. Update the images for player.html using a css sprite
2. Add an alternative web interface if possible

## Installer

1. Add file association tasks, see <https://trac.mpc-hc.org/ticket/2207>

## VSFilter

1. Use the defines from version.h for `MPC_COPYRIGHT_STR` in VSFilter
2. Define VSFilter's version numbers in a place accessible by its installer and VSFilter itself
3. Backport a few useful patches from those VSFilter forks; we need individual patches for that
   and we have to make sure that everything works right for mpc-hc itself
