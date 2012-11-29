/* This script is a local pre-commit hook script.
 * It's used to check whether the copyright year of modified files has been
 * bumped up to the current (2012) year.
 *
 * Taken from TortoiseSVN repository, adapted for MPC-HC by thevbm
 *
 * Only *.cpp and *.h files are checked
 *
 * Set the local hook scripts like this (pre-commit hook):
 * WScript path/to/this/script/file.js
 * and set "Wait for the script to finish"
 */

var ForReading = 1;
var objArgs, num;

objArgs = WScript.Arguments;
num = objArgs.length;
if (num !== 4)
{
    WScript.Echo("Usage: [CScript | WScript] checkyear.js path/to/pathsfile depth path/to/messagefile path/to/CWD");
    WScript.Quit(1);
}

var re = /^(.+)(2006-2012)(.+)/;
var basere = /^(.+)see\sAuthors.txt/;
var filere = /(\.cpp$)|(\.h$)/;
var found = true;
var fs, a, rv, r;
fs = new ActiveXObject("Scripting.FileSystemObject");
// remove the quotes
var files = readPaths(objArgs(0));
// going backwards with while is believed to be faster
var fileindex = files.length;
var errormsg = "";

while (fileindex--)
{
    var f = files[fileindex];
    if (f.match(filere) !== null)
    {
        if (fs.FileExists(f))
        {
            a = fs.OpenTextFile(f, ForReading, false);
            var copyrightFound = false;
            var yearFound = false;
            while ((!a.AtEndOfStream) && (!yearFound))
            {
                r =  a.ReadLine();
                rv = r.match(basere);
                if (rv !== null)
                {
                    rv = r.match(re);
                    if (rv !== null)
                        yearFound = true;

                    copyrightFound = true;
                }
            }
            a.Close();

            if (copyrightFound && (!yearFound))
            {
                if (errormsg !== "")
                    errormsg += "\n";
                errormsg += f;
                found = false;
            }
        }
    }
}

if (found === false)
{
    errormsg = "the file(s):\n" + errormsg + "\nhave not the correct copyright year!";
    WScript.stderr.writeLine(errormsg);
}

WScript.Quit(!found);


// readFileLines
function readPaths(path)
{
    var retPaths = new Array();
    var fs = new ActiveXObject("Scripting.FileSystemObject");
    if (fs.FileExists(path))
    {
        var a = fs.OpenTextFile(path, ForReading);
        while (!a.AtEndOfStream)
        {
            var line = a.ReadLine();
            retPaths.push(line);
        }
        a.Close();
    }
    return retPaths;
}
