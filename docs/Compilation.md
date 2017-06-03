# Compilation instructions

For up to date instructions on how to compile mpc-hc visit the wiki page: <https://trac.mpc-hc.org/wiki/How_to_compile_the_MPC>


## Part A: Preparing the Visual Studio environment

### Visual Studio 2015

1. Install Visual C++ 2015, part of Visual Studio 2015 (any edition will work fine)
2. Make sure you have installed all available updates from Microsoft Update
3. Install the DirectX SDK (June 2010) → <https://go.microsoft.com/fwlink/?LinkID=71193>


## Part B: Preparing the GCC environment

1. Download and extract **MSYS_MinGW-w64_GCC_710_x86-x64.7z** to **C:\MSYS** → <http://xhmikosr.1f0.de/tools/msys/MSYS_MinGW-w64_GCC_710_x86-x64.7z>.
   For the components and their version see <http://xhmikosr.1f0.de/tools/msys/MSYS_MinGW-w64_GCC_710_x86-x64_components.txt>
2. Create a file named **build.user.bat** in **C:\mpc-hc** containing the following entries, adapted for your system:

    ```bat
    @ECHO OFF
    SET "MPCHC_MSYS=C:\MSYS"
    SET "MPCHC_MINGW32=%MPCHC_MSYS%\mingw"
    SET "MPCHC_MINGW64=%MPCHC_MINGW32%"
    REM Git is optional to set if you chose to add it in PATH when installing it
    SET "MPCHC_GIT=C:\Program Files\Git"
    REM Optional, if you plan to modify the translations, install Python 2.7 or set the variable to its path
    SET "MPCHC_PYTHON=C:\Python27"
    ```

### NOTES

* If you installed the MSYS/MinGW package in another directory you will have to use that path in the previous steps.
* If you don't have Git installed then the revision number will be a hard-coded one, like 1.6.3.0.


## Part C: Downloading and compiling the MPC-HC source

1. Use Git to clone MPC-HC's repository to **C:\mpc-hc** (or anywhere else you like).

    1. Download Git from <https://git-for-windows.github.io/>
    2. Run:

        ```text
        git clone --recursive https://github.com/mpc-hc/mpc-hc.git
        ```

        or

        ```text
        git clone https://github.com/mpc-hc/mpc-hc.git
        git submodule update --init --recursive
        ```

        If a submodule update fails, try running:

        ```text
        git submodule foreach --recursive git fetch --tags
        ```

        then run the update again

        ```text
        git submodule update --init --recursive
        ```

        Note that you can add `-b master` to the `git clone` command if you want to get the latest
        stable version instead of the latest development version
2. Open the solution file **C:\mpc-hc\mpc-hc.sln**.
   Change the solution's configuration to **Release** (in the toolbar).
3. Press **F7** to build the solution.
4. You now have **mpc-hc.exe** under **C:\mpc-hc\bin\mpc-hc_x86**
5. Open the solution file **C:\mpc-hc\mpciconlib.sln**
6. Press **F7** to build the solution.
7. You now have **mpciconlib.dll** under **C:\mpc-hc\bin\mpc-hc_x86**
8. Open the solution file **C:\mpc-hc\mpcresources.sln**
9. Build **BuildAll** project.
10. You now have **mpcresources.XX.dll** under **C:\mpc-hc\bin\mpc-hc_x86\Lang**

Alternatively, you can use **build.bat** that can build everything for you (run: `build.bat help` for more info)


## Part D: Building the installer

Download Inno Setup Unicode v5.5.9 or newer from <http://www.jrsoftware.org/isdl.php>.
Install everything and then go to **C:\mpc-hc\distrib**, open **mpc-hc_setup.iss** with Inno Setup,
read the first comments in the script and compile it.

### NOTES

* **build.bat** can build the installer by using the **installer** or the **packages** switch.
* Use Inno Setup's built-in IDE if you want to edit the iss file.
