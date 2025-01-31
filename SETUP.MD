# Compilation Process
This document details the process of starting with the unmodified data as part of the Frogger 2 Development Archives and putting together a workspace which can build the Frogger 2 PC version.  
This is **NOT** the guide for compiling the PC version, but instead the documentation for how this repository was created.  

## Terminology
**Frogger 2 Development Archives:**  
If you're just looking at this git repository on its own, it is part of a much larger release of Frogger 2 development backups.  
This release is public and is approximately 60-80 GB. The repositories such as this one have been created to simplify code access, so anyone who wants code access can just look at a much smaller repository which has been setup to easily compile.  
A copy of this repository is found inside of the release as well.  
In order to follow this guide, you will need to have these backups downloaded, though it may be more convenient to only extract what you need from the .zip file.  
You can get this file from archive.org, and hopefully from other mirrors. At the time of writing this, the release is not public yet so I can't provide a download link.  

**Tools Folder:**  
The tools folder is located in the release at `\Misc\Gathered Tools\`, and contains many utilities for working with the files and data present in the release.  

**Tape:**
There was a data tape in the Frogger 2 development archives which contained the "End of Project" backup, much of the most important data.  
Despite this being a tape, it has been recovered and made into a .zip file, making it very simple for you to access the files inside.  
This zip file is found at `\End of Project Tape Backup\Frogger 2 EOP Backup 18042001.zip` in the development archives.  

## Steps

### Before Getting Started
Before getting started, consider where you want to set this up. This process can be done in a Windows XP VM, but this process also has been tested on Windows 10.  
A VM is advised if you plan to use the Visual C++ installer, as only the other option has been tested on Windows 10.  

### Step 1 (Microsoft Visual C++ 6.0)
Microsoft Visual C++ 6.0 is required to build Frogger 2, and we have two ways of obtaining it.  

**Option 1 (Less Work, Uses Backup):**  
Inside of the tape backup there is a backup of Visual C++ 6.0 which you can just extract.  
This is the recommended option on modern Windows, to avoid potential compatibility issues with the Visual C++ installer.  
Extract the folder `\\BLITZONE\DATA\TeamSpirit\pcversion\DevStudio\` from the tape to an empty folder called `DevStudio` anywhere you like.  

If you're running on a modern system you need to put `msvcp50.dll` into the `DevStudio\SharedIDE\bin\` folder.  
This file is created by microsoft and should be possible to find online.  
It is also backed up on one of the Chicken Run tapes at `ChickenRun\Psx\Misc_Src_Data\Gizmo\msvcp50.dll`.  

**Option 2 (More Work, Use the Installer):**  
Two separate copies of Microsoft Visual C++ 6.0 are available in the 
PowerISO + ISOBuster are included in the tools folder, but Windows includes built-in mounting capabilities just by right-clicking the file.  
If you're using a VM, your VM software will almost certainly contain the ability to mount CD images too.  
Mount the first CD image, and follow the instructions to install Visual C++. If this option fails, consider trying Option #1 instead.  

### Step 2 (DirectX 7 SDK)
This step is mandatory, even if you chose **Option 1** in the previous step.  
Skipping this step would cause many DirectX errors about symbols not being found during compilation.  
Locate the DirectX 7 SDK in the tools folder, there should be a single exe called "dx7sdk-7001.exe".  
This executable is a self-extracting archive, and will extract its contents wherever you tell it to.  
Once you've extracted these files, there are two folders of note: "include" and "lib".  

Next, locate your Dev Studio Install folder, based on which option you chose in the previous step:  
**Option 1:** `DevStudio\VC\`
**Option 2:** The location you installed 

To confirm you're viewing the right folder, verify you see both two folders named "lib" and "include".  
Copy those folders from the DirectX SDK folder here, overwriting files.  
Do not delete any of the existing folders, the existing folders contain files that the DirectX files are missing.  

### Step 3 (Setup Folders)
Unfortunately, Frogger 2's PC build setup was not preserved very well in backups.  
There are some modifications that need to be made to make it buildable.  
Start by choosing how you will get your source code. The code necessary is available in the Visual SourceSafe backup found at `\\BLITZONE\DATA\TeamSpirit\VSS Backup\FINAL 18042001.ssa` in the tape backup.  
If you have Visual C++ 6.0 installed, you also have Visual SourceSafe, and can import the .ssa using the "Visual SourceSafe 6.0 Admin" program.  
However, [this converted repository](https://github.com/HighwayFrogs/frogger2-vss) is recommended because the files are readily accessible without legacy tech.   
That repository is also available in the development archives at `\Code Backups\Modern Setups by Kneesnap\frogger2-vss.zip`.  

**Folder Structure:**  
Create a new empty folder. I recommend `frogger2-pc`, but you can call it anything. This will be your project folder.  
IMPORTANT: Dev Studio requires \r\n (CRLF) line endings. If when opening a project it tries to load it as a makefile, check the line endings.  
The names of everything from this point (file names, folders, etc) are important and if not followed may cause problems.  
Inside your project folder:  
 - Create a folder called 'MDX', and fill with the contents of `$/teamSpirit/MDX Library`.  
 - Create a folder called 'Common', and fill with the contents of `$/teamSpirit/FroggerProject/Common`.  
 - Create a folder called 'Editor', and fill with the contents of `$/teamSpirit/FroggerProject/Editor`.  
 - Create a folder called 'PC', and fill with the contents of `$/teamSpirit/FroggerProject/PC`.  
 - Create a folder called 'PSX', and fill with the contents of `$/teamSpirit/FroggerProject/PSX`.  
 - Copy all of the files from `$/teamSpirit/FroggerProject/` except `atari.ico`.
 - Create a folder called 'gELF', and fill with the contents of `\\BLITZONE\DATA\TeamSpirit\Dreamcast\Work\gELF` in the tape backup. Duplicate gelf.lib as gelf.debug.lib.  
 - Create a folder called 'Bink', it will be setup separately.  
 - Create a folder called 'SoftStation', and move `SoftStation.h` and `SoftStation.lib` from the `MDX\` folder.  
 
**The Bink Folder:**  
This folder contains the utilities for the Bink video program.  
Unfortunately, the libraries and data were not included in any Frogger 2 backups, they were included in a Chicken Run backup, and may be possible to find online.  
On the last data session on the "Chicken Run End of Project #1" tape, is the file path `Bink SDK and tools\BINKSDK.EXE`.  
Running that executable will extract the files `BINK.H, RAD.H, binkw32.lib, and binkw32.dll`.  
Copy these files to your `bink\` folder, or copy them from this repository.  

**Common\enemies.c:**  
The file `Common\enemies.c` contains references to path `x:\teamspirit\pcversion\nmehack.txt` and `x:\teamspirit\pcversion\reactive.txt`.  
Replace these references with `..\nmehack.txt` and `..\reactive.txt` respectively.  
Then, put those text files in the project folder from `\\BLITZONE\DATA\TeamSpirit\pcversion\`.  

**Common\textover.c:**  
The file needs to `#include "training.h"`, else it is unable to resolve the static variable `train`.

**DLGPROC:**  
Much of the net code has calls to DialogBox(), and the fourth argument needs to be casted to `DLGPROC`.  
I suspect there an update to the IDE would fix it.  

**resource.rc:**  
If building on modern Windows, replace `#include "afxres.h"` with `#include <windows.h>` in resource.rc.    

### Step 4 (Update Project Files)
The project files aren't setup properly, and it is unknown why. Instead of trying to conform to the original organization, we're going to modify them to work with our new setup.  

**MDX\MDX.dsp:**  
```
Line 47:
# ADD CPP /nologo /GX /O2 /Ob2 /I "." /I "..\froggerii-b\common" /I "..\froggerii-b\pc" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
Replace With:
# ADD CPP /nologo /GX /O2 /Ob2 /I "." /I "..\common" /I "..\pc" /I "..\gelf" /I "..\softstation" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c


Line 67:
# ADD CPP /nologo /MTd /W3 /GX /Zi /Od /I "." /I "..\froggerii-b\common" /I "..\froggerii-b\pc" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FAcs /Fr /YX /FD /c
Replace With:
# ADD CPP /nologo /MTd /W3 /GX /Zi /Od /I "." /I "..\common" /I "..\pc" /I "..\gelf" /I "..\softstation" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FAcs /Fr /YX /FD /c
```

**FroggerProject.dsp:**  
```
LINE 47:
# ADD CPP /nologo /w /W0 /GX /O2 /Ob2 /I "pc" /I "common" /I "editor" /I "\sdk\bink" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "PC_VERSION" /D "FINAL_MASTER" /D "USE_BINK_VIDEO" /YX /FD /c
Replace With:
# ADD CPP /nologo /w /W0 /GX /O2 /Ob2 /I "pc" /I "common" /I "editor" /I "bink" /I "gelf" /I "mdx" /I "softstation" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "PC_VERSION" /D "FINAL_MASTER" /D "USE_BINK_VIDEO" /YX /FD /c

LINE 57:
# ADD LINK32 dxguid.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib uuid.lib gelf.lib mdx.lib softstation.lib ddraw.lib d3dim.lib dsound.lib dinput.lib winmm.lib comctl32.lib softstation.lib libc.lib libci.lib msvcrt.lib oldnames.lib binkw32.lib dplay.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib /out:"Release/Frogger2.exe" /libpath:"..\mdx" /libpath:"\sdk\bink"
Replace With:
# ADD LINK32 dxguid.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib uuid.lib gelf.lib mdx.lib softstation.lib ddraw.lib d3dim.lib dsound.lib dinput.lib winmm.lib comctl32.lib softstation.lib libc.lib libci.lib msvcrt.lib oldnames.lib binkw32.lib dplay.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib /out:"Release/Frogger2.exe" /libpath:"bink" /libpath:"gelf" /libpath:"mdx" /libpath:"softstation"

LINE 74:
# ADD CPP /nologo /MTd /W2 /Gm /GX /ZI /Od /I "pc" /I "common" /I "editor" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "PC_VERSION" /FR /YX /FD /GZ /c
Replace With:
# ADD CPP /nologo /MTd /W2 /Gm /GX /ZI /Od /I "pc" /I "common" /I "editor" /I "gelf" /I "mdx" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "PC_VERSION" /FR /YX /FD /GZ /c

LINE 84:
# ADD LINK32 dsound.lib winmm.lib dxguid.lib ddraw.lib dinput.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib gelf.debug.lib mdx.lib softstation.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libc" /nodefaultlib:"libcd" /libpath:"..\mdx\Blank_Library___Win32_Debug"
Replace With:
# ADD LINK32 dsound.lib winmm.lib dxguid.lib ddraw.lib dinput.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib gelf.debug.lib mdx.lib softstation.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libc" /nodefaultlib:"libcd" /libpath:"gelf" /libpath:"mdx\Debug" /libpath:"softstation"

LINE 101:
# ADD BASE CPP /nologo /Gm /GX /ZI /Od /I "pc" /I "common" /I "editor" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "PC_VERSION" /FR /YX /FD /GZ /c
Replace With:
# ADD BASE CPP /nologo /Gm /GX /ZI /Od /I "pc" /I "common" /I "editor" /I "gelf" /I "mdx" /I "softstation" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "PC_VERSION" /FR /YX /FD /GZ /c

LINE 102:
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "pc" /I "common" /I "editor" /I "\sdk\bink" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "PC_VERSION" /D "USE_BINK_VIDEO" /FR /YX /FD /GZ /c
Replace With:
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "pc" /I "common" /I "editor" /I "bink" /I "gelf" /I "mdx" /I "softstation" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "PC_VERSION" /D "USE_BINK_VIDEO" /FR /YX /FD /GZ /c

LINE 111:
# ADD BASE LINK32 dsound.lib winmm.lib dxguid.lib ddraw.lib dinput.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib gelf.debug.lib mdx.lib softstation.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libc" /libpath:"..\mdx\Blank_Library___Win32_Debug"
Replace With:
# ADD BASE LINK32 dsound.lib winmm.lib dxguid.lib ddraw.lib dinput.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib gelf.debug.lib mdx.lib softstation.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libc" /libpath:"gelf" /libpath:"mdx\Debug" /libpath:"softstation"

LINE 113:
# ADD LINK32 dsound.lib winmm.lib dxguid.lib ddraw.lib dinput.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib gelf.debug.lib mdx.lib softstation.lib binkw32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libc" /nodefaultlib:"libcd" /libpath:"..\mdx\Blank_Library___Win32_Debug" /libpath:"\sdk\bink"
Replace With:
# ADD LINK32 dsound.lib winmm.lib dxguid.lib ddraw.lib dinput.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib gelf.debug.lib mdx.lib softstation.lib binkw32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libc" /nodefaultlib:"libcd" /libpath:"bink" /libpath:"gelf" /libpath:"mdx\Debug" /libpath:"softstation"
```

### Step 5 (Build Project)
Open Dev Studio from `DevStudio\SharedIDE\bin\MSDEV.EXE` or `{InstallFolder}\Common\MSDev98\Bin\msdev.exe`.  
1) File > Open Workspace  
2) Select `Projects (*.dsp)` from the "Files of Type" dropdown.  
3) Open "{project folder}\MDX\MDX.dsp".  
4) Build > Set Active Configuration... > Set to the "Release" version.  
5) Build > Build mdx.lib -> Should successfully build mdx.lib in the MDX\ folder.  
6) File > Open Workspace -> Open FroggerProject.dsp  
7) Select whichever build configuration you'd like to build, and you should be able to build it.  
8) You should now be able to build a Windows version of Frogger 2 from source.

### Step 6, Optional (Cleanup)
**Remove PSX Code:**  
Lets first cleanup by deleting the PSX folder in Dev Studio, then deleting it from the filesystem.  
We needed to copy it earlier since the project file contained references to the files in this folder. But, we don't need them anymore.  

### Step 7, Optional (Upgrade to modern Visual Studio)
Nobody wants an ancient IDE. Let's upgrade to modern Visual Studio.  
Unfortunately, Visual Studio 2022 no longer supports reading .dsp files.  
According to [the MSDN](https://learn.microsoft.com/en-us/cpp/porting/upgrading-projects-from-earlier-versions-of-visual-cpp) ([archive](https://web.archive.org/web/20230727232131/https://learn.microsoft.com/en-us/cpp/porting/upgrading-projects-from-earlier-versions-of-visual-cpp?view=msvc-170)), Visual Studio 2019 is the last version capable of upgrading .dsp/.dsw files.  
So, that's what I'll use for this guide. I've bundled Visual Studio 2005 in the tools folder just in case it becomes impossible to get VS2019 in the future.  

1) File > Open Project / Solution  
2) Choose "VC++ Project Files (\*.vcxproj, \*.vcproj, \*.dsp, \*.mdp, ....)  
3) Open 'FroggerProject.dsp'
4) Approve one-way upgrade.
5) Right Click Solution > Add > Existing Project... > Select 'MDX\MDX.dsp'
6) Rename "Blank Library" to MDX.
7) Update each build configuration to enable "Function Level Linking" in Properties > C/C++ > Code Generation > Enable Function Level Linking.  

The game will not be compileable in the state it's in here. You will need to fix the game up to compile again.  
There is no current setup to build with modern Visual Studio, though potentially VS could be hooked up to the 1997 compiler setup.   