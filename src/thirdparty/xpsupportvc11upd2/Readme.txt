Fix for Bug in Update2 for VS2012 which causes apps built with this to fail on XP.
Uninstalling Update2 is unfortunately not an option since it removes
ATL completely, and not even a complete reinstall of VS2012 fixes this.

Taken from:
http://tedwvc.wordpress.com/2013/04/14/how-to-get-visual-c-2012-update-2-statically-linked-applications-to-run-on-windows-xp/
