# Compilation instructions

## General Tools

First of all, install **Git for Windows** from <https://git-for-windows.github.io/>.
Choose `Use Git from the Windows command prompt`. This isn't mandatory, so if you choose
`Use Git from Git Bash only` make sure you set the `MPCHC_GIT` variable in `build.user.bat`.


## Part A: Preparing the Visual Studio environment

### Visual Studio 2017

1. Install Visual C++, part of Visual Studio (any edition will work fine).
   Make sure to select **Windows 8.1 SDK** and **MFC and ATL support** and **Windows Universal CRT SDK** during installation.
2. Make sure you have installed all available updates from Microsoft Update
3. Install the DirectX SDK (June 2010) → <https://go.microsoft.com/fwlink/?LinkID=71193>


## Part B: Preparing the GCC environment

1. Download MSYS2 from <http://www.msys2.org/>.
   If you are on a 64-bit Operating System, which you should be, get the 64-bit version.
2. Install it on **`C:\MSYS\`**. You can always install it somewhere else, but these instructions
   assume the aforementioned place.
3. Run `msys2_shell.bat` (if you didn't use the installer you must restart MSYS2 after first run).
4. Install the needed software by running these commands:
   ```text
   pacman -Syu
   pacman -Su
   pacman -S make pkg-config
   ```
   Note that this is the bare minimum, you can install more packages that can be useful to you.
5. Download YASM and save it as **yasm.exe** in **`C:\MSYS\usr\bin`**:
   * 32-bit: <http://www.tortall.net/projects/yasm/releases/yasm-1.3.0-win32.exe>
   * 64-bit: <http://www.tortall.net/projects/yasm/releases/yasm-1.3.0-win64.exe> (Recommended)
6. Download and extract MinGW to **`C:\MSYS\mingw\`** from <http://files.1f0.de/mingw/>
   (you might use the one that ships with MSYS2, but we recommend this one)
7. Create a file named **build.user.bat** in **`C:\mpc-hc\`** containing the following entries,
   adapted for your system:

    ```bat
    @ECHO OFF
    SET "MSYSTEM=MINGW32"
    SET "MPCHC_MSYS=C:\MSYS"
    SET "MPCHC_MINGW32=%MPCHC_MSYS%\mingw"
    SET "MPCHC_MINGW64=%MPCHC_MINGW32%"
    REM You can set `MSYS2_PATH_TYPE` here or in environment variables so that Git is properly added to your `PATH`
    REM SET "MSYS2_PATH_TYPE=inherit"
    REM `MPCHC_GIT` is optional to set if you chose to add it in `PATH` when installing it and have set `MSYS2_PATH_TYPE`
    SET "MPCHC_GIT=C:\Program Files\Git"
    REM Optional, if you plan to modify the translations, install Python 2.7 or set the variable to its path
    SET "MPCHC_PYTHON=C:\Python27"
    REM Optional, If you want to customize the Windows SDK version used, set the variable
    SET "MPCHC_WINSDK_VER=8.1"
    ```

### NOTES

* For Visual Studio 2017, we will try to detect the VS installation path automatically.
  If that fails you need to specify the installation path yourself. For example:
  ```bat
  SET "MPCHC_VS_PATH=%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\"
  ```
* If you installed the MSYS/MinGW package in another directory you will have to use that path in the previous steps.
* If you don't have Git installed then the revision number will be a hard-coded one, like 1.6.3.0.


## Part C: Downloading and compiling the MPC-HC source

1. Use Git to clone MPC-HC's repository to **C:\mpc-hc** (or anywhere else you like).

    1. Download Git from <https://git-for-windows.github.io/>
    2. Run:

        ```text
        git clone --recursive https://github.com/clsid2/mpc-hc.git
        ```

        or

        ```text
        git clone https://github.com/clsid2/mpc-hc.git
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
* Use Inno Setup's built-in IDE if you want to edit the iss file and don't change its encoding since it can break easily.
