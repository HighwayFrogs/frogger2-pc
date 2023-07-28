![Frogger 2 Logo](/_repo/Frogger2Logo.png)
# Frogger 2: Swampy's Revenge (Dreamcast)  
This is a backup of Frogger 2's official PC source code from the [VSS repository](https://github.com/HighwayFrogs/frogger2-vss).  
It does not build a byte for byte match of the retail PC version because 
**Developer:** Blitz Games  

## How to compile and run?  
Follow [this guide](/SETUP.MD) up until you have DevStudio setup, then after cloning this repository run:
Open the Solution with Visual Studio & Build Project.  
The "Release" build configuration does not work, because we only have the debug gelf.lib files, which use `_CrtDbgReport`, a function unavailable in release mode in the MS compiler.  
Perhaps another Blitz Games archive could contain a proper version.  

## I want to use a new Visual Studio version:
Yeah, don't blame you. Unfortunately, DirectPlay was removed around 2007, so any reference to `dplay.h` or `dplobby.h` are impossible to compile anymore.  
Visual Studio 2019 is the last Visual Studio version capable of upgrading .dsp/.dsw projects to modern Visual Studio formats.  
So, in order to use new Visual Studio, you will need to either replace DirectPlay with something modern or find some way to utilize DirectPlay with a modern compiler.  
Scroll to the bottom of [the guide](/SETUP.MD) to get started.  

## Asset Information
Assets are not included, because at the time of writing the original assets and tools are available in the end of project tape backup.  
Playing the game is also as simple as replacing the exe from a retail PC build with your own, and does not require building any CD images, so there is little need to include them.  

Preserved by [Highway Frogs](https://highwayfrogs.net/)