// this script is a local pre-commit hook script.
// Used to check whether the copyright year of modified files has been bumped
// up to the current (2012) year.
// Taken from TortoiseSVN repository, adapted for MPC-HC by thevbm
//
// Only *.cpp and *.h files are checked
//
// Set the local hook scripts like this (pre-commit hook):
// WScript path/to/this/script/file.js
// and set "Wait for the script to finish"

var objArgs, num;

objArgs = WScript.Arguments;
num = objArgs.length;

if (num != 4)
{
    WScript.Echo("Usage: [CScript | WScript] checkyear.js path/to/pathsfile depth path/to/messagefile path/to/CWD");
    WScript.Quit(1);
}

var re = /^(.+)(2006-2012)(.+)/;
var basere = /^(.+)see\sAuthors.txt/;
var filere = /(\.cpp$)|(\.h$)/;
var found = true;
var fs, a, ForAppending, rv, r;
ForReading = 1;
fs = new ActiveXObject("Scripting.FileSystemObject");
// remove the quotes
var files = readPaths(objArgs(0));
var fileindex=0;
var errormsg = "";

while (fileindex < files.length)
{
    var f = files[fileindex];
    if (f.match(filere) != null)
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
                if (rv != null)
                {
                    rv = r.match(re);
                    if (rv != null)
                    {
                        yearFound = true;
                     }

                    copyrightFound = true;
                }
            }
            a.Close();

            if (copyrightFound && (!yearFound))
            {
                if (errormsg != "")
                {
                    errormsg += "\n";
                }
                errormsg += f;
                found = false;
            }
        }
    }
    fileindex+=1;
}

if (found == false)
{
    errormsg = "the file(s):\n" + errormsg + "\nhave not the correct copyright year!";
    WScript.stderr.writeLine(errormsg);
}

WScript.Quit(!found);

function readPaths(path)
{
    var retPaths = new Array();
    var fs = new ActiveXObject("Scripting.FileSystemObject");

    if (fs.FileExists(path))
    {
        var a = fs.OpenTextFile(path, 1, false);
        var i = 0;
        while (!a.AtEndOfStream)
        {
            var line = a.ReadLine();
            retPaths[i] = line;
            i = i + 1;
        }
        a.Close();
    }
    return retPaths;

}
