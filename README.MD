![Frogger 2 Logo](/_repo/Frogger2Logo.png)
# Frogger 2: Swampy's Revenge (PC)  
This is a backup of Frogger 2's official PC source code from the [VSS repository](https://github.com/HighwayFrogs/frogger2-vss).  
It does not build a byte for byte match of the retail PC version because 
**Developer:** Blitz Games  

## How was this repository created?  
The steps for remaking this repository can be found at [this guide](/SETUP.MD).  
These are NOT the steps you should follow if you only wish to compile the game.  

## I want to use a new Visual Studio version:
Yeah, don't blame you. Unfortunately, DirectPlay was removed around 2007, so any reference to `dplay.h` or `dplobby.h` are impossible to compile anymore.  
Visual Studio 2019 is the last Visual Studio version capable of upgrading .dsp/.dsw projects to modern Visual Studio formats.  
So, in order to use new Visual Studio, you will need to either replace DirectPlay with something modern or find some way to utilize DirectPlay with a modern compiler.  
Scroll to the bottom of [the guide](/SETUP.MD) to get started for something like Visual Studio 2005. No methods are currently available for anything later than that.  

## Asset Information
Assets are not included, because the explanation of how to manage them is much larger than what belongs here.  
Playing the game is also as simple as replacing the exe from a retail PC build with your own, and does not require building any CD images, so there is little need to include them.  

## Where's the enhanced PC port source code?
Check the `fixes` branch.  

## How to compile and run?  
Open the Solution with Visual Studio & Build Project.  
The "Release" build configuration does not work, because we only have the debug gelf.lib files, which use `_CrtDbgReport`, a function unavailable in release mode in the MS compiler.  
Perhaps another Blitz Games archive could contain a proper version.  

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

### Step 3 (Build Project)
Open Dev Studio from `DevStudio\SharedIDE\bin\MSDEV.EXE` or `{InstallFolder}\Common\MSDev98\Bin\msdev.exe`.  
1) File > Open Workspace  
2) Select `Projects (*.dsp)` from the "Files of Type" dropdown.  
3) Open "{project folder}\MDX\MDX.dsp".  
4) Build > Set Active Configuration... > Set to the "Debug" version.  
5) Build > Build mdx.lib -> Should successfully build mdx.lib in the MDX\ folder.  
6) File > Open Workspace -> Open FroggerProject.dsp  
7) Select whichever build configuration you'd like to build, and you should be able to build it.  
8) You should now be able to build a Windows version of Frogger 2 from source.

Preserved by [Highway Frogs](https://highwayfrogs.net/)