# Microsoft Developer Studio Project File - Name="FroggerProject" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=FroggerProject - Win32 Debug with Bink Video
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "FroggerProject.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "FroggerProject.mak"\
 CFG="FroggerProject - Win32 Debug with Bink Video"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "FroggerProject - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "FroggerProject - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "FroggerProject - Win32 Debug with Bink Video" (based on\
 "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""$/teamSpirit/FroggerProject", ZQMBAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "FroggerProject - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MLd /w /W0 /GX /O2 /Ob2 /I "pc" /I "common" /I "editor" /I "bink" /I "gelf" /I "mdx" /I "softstation" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "PC_VERSION" /D "FINAL_MASTER" /D "USE_BINK_VIDEO" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 dxguid.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib uuid.lib gelf.lib mdx.lib softstation.lib ddraw.lib d3dim.lib dsound.lib dinput.lib winmm.lib comctl32.lib softstation.lib libc.lib libci.lib msvcrt.lib oldnames.lib binkw32.lib dplay.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib /out:"Release/Frogger2.exe" /libpath:"mdx" /libpath:"bink" /libpath:"gelf" /libpath:"softstation"
# SUBTRACT LINK32 /verbose /debug

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /ZI /GZ /c
# ADD CPP /nologo /MTd /W2 /GX /Od /I "pc" /I "common" /I "editor" /I "gelf" /I "mdx" /I "softstation" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "PC_VERSION" /FR /YX /FD /ZI /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 dsound.lib winmm.lib dxguid.lib ddraw.lib dinput.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib gelf.debug.lib mdx.lib softstation.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libc" /nodefaultlib:"libcd" /libpath:"mdx\Debug" /libpath:"gelf" /libpath:"softstation"
# SUBTRACT LINK32 /profile

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug with Bink Video"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "FroggerProject___Win32_Debug_with_Bink_Video"
# PROP BASE Intermediate_Dir "FroggerProject___Win32_Debug_with_Bink_Video"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugBink"
# PROP Intermediate_Dir "DebugBink"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /GX /Od /I "pc" /I "common" /I "editor" /I "gelf" /I "mdx" /I "softstation" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "PC_VERSION" /FR /YX /FD /ZI /GZ /c
# ADD CPP /nologo /MTd /W3 /GX /Zi /Od /I "pc" /I "common" /I "editor" /I "bink" /I "gelf" /I "mdx" /I "softstation" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "PC_VERSION" /D "USE_BINK_VIDEO" /FR /YX /FD /ZI /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 dsound.lib winmm.lib dxguid.lib ddraw.lib dinput.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib gelf.debug.lib mdx.lib softstation.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libc" /libpath:"gelf" /libpath:"mdx\Debug" /libpath:"softstation"
# SUBTRACT BASE LINK32 /profile
# ADD LINK32 dsound.lib winmm.lib dxguid.lib ddraw.lib dinput.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib gelf.debug.lib mdx.lib softstation.lib binkw32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libc" /nodefaultlib:"libcd" /libpath:"bink" /libpath:"gelf" /libpath:"mdx\Debug" /libpath:"softstation"
# SUBTRACT LINK32 /profile

!ENDIF 

# Begin Target

# Name "FroggerProject - Win32 Release"
# Name "FroggerProject - Win32 Debug"
# Name "FroggerProject - Win32 Debug with Bink Video"
# Begin Group "Common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Common\actor2.c
# End Source File
# Begin Source File

SOURCE=.\Common\actor2.h
# End Source File
# Begin Source File

SOURCE=.\Common\anim.c
# End Source File
# Begin Source File

SOURCE=.\Common\anim.h
# End Source File
# Begin Source File

SOURCE=.\Common\babyfrog.c
# End Source File
# Begin Source File

SOURCE=.\Common\babyfrog.h
# End Source File
# Begin Source File

SOURCE=.\Common\bbtimer.c
# End Source File
# Begin Source File

SOURCE=.\Common\bbtimer.h
# End Source File
# Begin Source File

SOURCE=.\Common\cam.c
# End Source File
# Begin Source File

SOURCE=.\Common\Cam.h
# End Source File
# Begin Source File

SOURCE=.\Common\codes.h
# End Source File
# Begin Source File

SOURCE=.\Common\collect.c
# End Source File
# Begin Source File

SOURCE=.\Common\collect.h
# End Source File
# Begin Source File

SOURCE=.\Common\crc32.c
# End Source File
# Begin Source File

SOURCE=.\Common\crc32.h
# End Source File
# Begin Source File

SOURCE=.\Common\decomp.h
# End Source File
# Begin Source File

SOURCE=.\Common\DefData.c
# End Source File
# Begin Source File

SOURCE=.\Common\DefData.h
# End Source File
# Begin Source File

SOURCE=.\Common\define.h
# End Source File
# Begin Source File

SOURCE=.\Common\E3_Demo.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Common\E3_Demo.h
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Common\enemies.c
# End Source File
# Begin Source File

SOURCE=.\Common\enemies.h
# End Source File
# Begin Source File

SOURCE=.\Common\evenfunc.c
# End Source File
# Begin Source File

SOURCE=.\Common\evenfunc.h
# End Source File
# Begin Source File

SOURCE=.\Common\event.c
# End Source File
# Begin Source File

SOURCE=.\Common\event.h
# End Source File
# Begin Source File

SOURCE=.\Common\fixed.c
# End Source File
# Begin Source File

SOURCE=.\Common\fixed.h
# End Source File
# Begin Source File

SOURCE=.\Common\flatpack.c
# End Source File
# Begin Source File

SOURCE=.\Common\flatpack.h
# End Source File
# Begin Source File

SOURCE=.\Common\frogger.c
# End Source File
# Begin Source File

SOURCE=.\Common\frogger.h
# End Source File
# Begin Source File

SOURCE=.\Common\frogmove.c
# End Source File
# Begin Source File

SOURCE=.\Common\frogmove.h
# End Source File
# Begin Source File

SOURCE=.\Common\frontend.c
# End Source File
# Begin Source File

SOURCE=.\Common\frontend.h
# End Source File
# Begin Source File

SOURCE=.\Common\game.c
# End Source File
# Begin Source File

SOURCE=.\Common\game.h
# End Source File
# Begin Source File

SOURCE=.\Common\general.c
# End Source File
# Begin Source File

SOURCE=.\Common\general.h
# End Source File
# Begin Source File

SOURCE=.\Common\hud.c
# End Source File
# Begin Source File

SOURCE=.\Common\hud.h
# End Source File
# Begin Source File

SOURCE=.\Common\lang.h
# End Source File
# Begin Source File

SOURCE=.\Common\language.c
# End Source File
# Begin Source File

SOURCE=.\Common\language.h
# End Source File
# Begin Source File

SOURCE=.\Common\layout.c
# End Source File
# Begin Source File

SOURCE=.\Common\layout.h
# End Source File
# Begin Source File

SOURCE=.\Common\levplay.c
# End Source File
# Begin Source File

SOURCE=.\Common\levplay.h
# End Source File
# Begin Source File

SOURCE=.\Common\map.c
# End Source File
# Begin Source File

SOURCE=.\Common\map.h
# End Source File
# Begin Source File

SOURCE=.\Common\maths.c
# End Source File
# Begin Source File

SOURCE=.\Common\maths.h
# End Source File
# Begin Source File

SOURCE=.\Common\memload.c
# End Source File
# Begin Source File

SOURCE=.\Common\memload.h
# End Source File
# Begin Source File

SOURCE=.\Common\menus.c
# End Source File
# Begin Source File

SOURCE=.\Common\menus.h
# End Source File
# Begin Source File

SOURCE=.\Common\multi.c
# End Source File
# Begin Source File

SOURCE=.\Common\multi.h
# End Source File
# Begin Source File

SOURCE=.\Common\newpsx.c
# End Source File
# Begin Source File

SOURCE=.\Common\newpsx.h
# End Source File
# Begin Source File

SOURCE=.\Common\options.c
# End Source File
# Begin Source File

SOURCE=.\Common\options.h
# End Source File
# Begin Source File

SOURCE=.\Common\overlays.c
# End Source File
# Begin Source File

SOURCE=.\Common\overlays.h
# End Source File
# Begin Source File

SOURCE=.\Common\particle.c
# End Source File
# Begin Source File

SOURCE=.\Common\particle.h
# End Source File
# Begin Source File

SOURCE=.\Common\path.c
# End Source File
# Begin Source File

SOURCE=.\Common\path.h
# End Source File
# Begin Source File

SOURCE=.\Common\platform.c
# End Source File
# Begin Source File

SOURCE=.\Common\platform.h
# End Source File
# Begin Source File

SOURCE=.\Common\ptexture.c
# End Source File
# Begin Source File

SOURCE=.\Common\ptexture.h
# End Source File
# Begin Source File

SOURCE=.\Common\savegame.c
# End Source File
# Begin Source File

SOURCE=.\Common\savegame.h
# End Source File
# Begin Source File

SOURCE=.\Common\script.c
# End Source File
# Begin Source File

SOURCE=.\Common\script.h
# End Source File
# Begin Source File

SOURCE=.\Common\sndview.c
# End Source File
# Begin Source File

SOURCE=.\Common\sndview.h
# End Source File
# Begin Source File

SOURCE=.\Common\sonylibs.h
# End Source File
# Begin Source File

SOURCE=.\Common\specfx.c
# End Source File
# Begin Source File

SOURCE=.\Common\specfx.h
# End Source File
# Begin Source File

SOURCE=.\Common\sprite.c
# End Source File
# Begin Source File

SOURCE=.\Common\sprite.h
# End Source File
# Begin Source File

SOURCE=.\Common\story.c
# End Source File
# Begin Source File

SOURCE=.\Common\story.h
# End Source File
# Begin Source File

SOURCE=.\Common\textover.c
# End Source File
# Begin Source File

SOURCE=.\Common\textover.h
# End Source File
# Begin Source File

SOURCE=.\Common\tongue.c
# End Source File
# Begin Source File

SOURCE=.\Common\tongue.h
# End Source File
# Begin Source File

SOURCE=.\Common\training.c
# End Source File
# Begin Source File

SOURCE=.\Common\training.h
# End Source File
# Begin Source File

SOURCE=.\Common\types.c
# End Source File
# Begin Source File

SOURCE=.\Common\types.h
# End Source File
# Begin Source File

SOURCE=.\Common\types2d.h
# End Source File
# Begin Source File

SOURCE=.\Common\ultra64.h
# End Source File
# End Group
# Begin Group "PC"

# PROP Default_Filter ""
# Begin Group "net"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Pc\net\dputil.cpp
# End Source File
# Begin Source File

SOURCE=.\Pc\net\netchat.cpp
# End Source File
# Begin Source File

SOURCE=.\Pc\net\netchat.h
# End Source File
# Begin Source File

SOURCE=.\Pc\net\netconnect.cpp
# End Source File
# Begin Source File

SOURCE=.\Pc\net\netconnect.h
# End Source File
# Begin Source File

SOURCE=.\Pc\net\netgame.cpp
# End Source File
# Begin Source File

SOURCE=.\Pc\net\netgame.h
# End Source File
# Begin Source File

SOURCE=.\Pc\net\netrace.cpp
# End Source File
# Begin Source File

SOURCE=.\Pc\net\netrace.h
# End Source File
# Begin Source File

SOURCE=.\Pc\net\network.cpp
# End Source File
# Begin Source File

SOURCE=.\Pc\net\network.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Pc\actor.cpp
# End Source File
# Begin Source File

SOURCE=.\Pc\actor.h
# End Source File
# Begin Source File

SOURCE=.\Pc\backdrop.c
# End Source File
# Begin Source File

SOURCE=.\Pc\backdrop.h
# End Source File
# Begin Source File

SOURCE=.\Pc\banks.cpp
# End Source File
# Begin Source File

SOURCE=.\Pc\banks.h
# End Source File
# Begin Source File

SOURCE=.\PC\CDAudioManager.c
# End Source File
# Begin Source File

SOURCE=.\PC\CDAudioManager.h
# End Source File
# Begin Source File

SOURCE=.\Pc\controll.cpp
# End Source File
# Begin Source File

SOURCE=.\Pc\controll.h
# End Source File
# Begin Source File

SOURCE=.\Pc\drawloop.cpp
# End Source File
# Begin Source File

SOURCE=.\Pc\drawloop.h
# End Source File
# Begin Source File

SOURCE=.\Pc\dx_sound.cpp
# End Source File
# Begin Source File

SOURCE=.\Pc\dx_sound.h
# End Source File
# Begin Source File

SOURCE=.\Pc\fadeout.cpp
# End Source File
# Begin Source File

SOURCE=.\Pc\fadeout.h
# End Source File
# Begin Source File

SOURCE=.\Pc\gtemac.h
# End Source File
# Begin Source File

SOURCE=.\Pc\inline_c.h
# End Source File
# Begin Source File

SOURCE=.\Pc\islfile.cpp
# End Source File
# Begin Source File

SOURCE=.\Pc\islfile.h
# End Source File
# Begin Source File

SOURCE=.\Pc\islfont.h
# End Source File
# Begin Source File

SOURCE=.\Pc\isllocal.c
# End Source File
# Begin Source File

SOURCE=.\Pc\isllocal.h
# End Source File
# Begin Source File

SOURCE=.\Pc\islmem.h
# End Source File
# Begin Source File

SOURCE=.\Pc\islpad.h
# End Source File
# Begin Source File

SOURCE=.\Pc\islpsi.h
# End Source File
# Begin Source File

SOURCE=.\Pc\isltex.cpp
# End Source File
# Begin Source File

SOURCE=.\Pc\isltex.h
# End Source File
# Begin Source File

SOURCE=.\Pc\islutil.cpp
# End Source File
# Begin Source File

SOURCE=.\Pc\islutil.h
# End Source File
# Begin Source File

SOURCE=.\Pc\libapi.h
# End Source File
# Begin Source File

SOURCE=.\Pc\libetc.h
# End Source File
# Begin Source File

SOURCE=.\Pc\libgpu.h
# End Source File
# Begin Source File

SOURCE=.\Pc\libgs.h
# End Source File
# Begin Source File

SOURCE=.\Pc\libgte.h
# End Source File
# Begin Source File

SOURCE=.\Pc\libsn.h
# End Source File
# Begin Source File

SOURCE=.\Pc\Main.cpp

!IF  "$(CFG)" == "FroggerProject - Win32 Release"

# ADD CPP /Zi

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug"

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug with Bink Video"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Pc\Main.h
# End Source File
# Begin Source File

SOURCE=.\Pc\net.h
# End Source File
# Begin Source File

SOURCE=.\Pc\pcaudio.c
# End Source File
# Begin Source File

SOURCE=.\Pc\pcaudio.h
# End Source File
# Begin Source File

SOURCE=.\Pc\pcgfx.c
# End Source File
# Begin Source File

SOURCE=.\Pc\pcgfx.h
# End Source File
# Begin Source File

SOURCE=.\Pc\pcmisc.cpp
# End Source File
# Begin Source File

SOURCE=.\Pc\pcmisc.h
# End Source File
# Begin Source File

SOURCE=.\Pc\pcsprite.c
# End Source File
# Begin Source File

SOURCE=.\Pc\pcsprite.h
# End Source File
# Begin Source File

SOURCE=.\Pc\shell.h
# End Source File
# Begin Source File

SOURCE=.\Pc\video.cpp
# End Source File
# Begin Source File

SOURCE=.\Pc\video.h
# End Source File
# End Group
# Begin Group "Editor"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Editor\edcamera.c

!IF  "$(CFG)" == "FroggerProject - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug"

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug with Bink Video"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Editor\edcamera.h

!IF  "$(CFG)" == "FroggerProject - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug"

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug with Bink Video"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Editor\edcommand.c

!IF  "$(CFG)" == "FroggerProject - Win32 Release"

# PROP Exclude_From_Build 1
# ADD CPP /Ob1

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug"

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug with Bink Video"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Editor\edcommand.h

!IF  "$(CFG)" == "FroggerProject - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug"

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug with Bink Video"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Editor\edcreate.c

!IF  "$(CFG)" == "FroggerProject - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug"

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug with Bink Video"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Editor\edcreate.h

!IF  "$(CFG)" == "FroggerProject - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug"

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug with Bink Video"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Editor\edglobals.c

!IF  "$(CFG)" == "FroggerProject - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug"

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug with Bink Video"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Editor\edglobals.h

!IF  "$(CFG)" == "FroggerProject - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug"

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug with Bink Video"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Editor\editdefs.c

!IF  "$(CFG)" == "FroggerProject - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug"

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug with Bink Video"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Editor\editdefs.h

!IF  "$(CFG)" == "FroggerProject - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug"

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug with Bink Video"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Editor\editfile.c

!IF  "$(CFG)" == "FroggerProject - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug"

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug with Bink Video"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Editor\editfile.h

!IF  "$(CFG)" == "FroggerProject - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug"

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug with Bink Video"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Editor\editor.c

!IF  "$(CFG)" == "FroggerProject - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug"

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug with Bink Video"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Editor\editor.h

!IF  "$(CFG)" == "FroggerProject - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug"

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug with Bink Video"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Editor\edittypes.h

!IF  "$(CFG)" == "FroggerProject - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug"

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug with Bink Video"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Editor\edmaths.c

!IF  "$(CFG)" == "FroggerProject - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug"

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug with Bink Video"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Editor\edmaths.h

!IF  "$(CFG)" == "FroggerProject - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug"

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug with Bink Video"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Editor\edtoolbars.c

!IF  "$(CFG)" == "FroggerProject - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug"

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug with Bink Video"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Editor\edtoolbars.h

!IF  "$(CFG)" == "FroggerProject - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug"

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug with Bink Video"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\Psx\acostab.s
# End Source File
# Begin Source File

SOURCE=.\Psx\asm.s
# End Source File
# Begin Source File

SOURCE=.\blitz.ico
# End Source File
# Begin Source File

SOURCE=.\Psx\bluebaby.pal
# End Source File
# Begin Source File

SOURCE=.\flog.bmp
# End Source File
# Begin Source File

SOURCE=.\frogger2.ico
# End Source File
# Begin Source File

SOURCE=.\frogger2site.ico
# End Source File
# Begin Source File

SOURCE=.\games.ico
# End Source File
# Begin Source File

SOURCE=.\Psx\gametext.h
# End Source File
# Begin Source File

SOURCE=.\Psx\greenbaby.pal
# End Source File
# Begin Source File

SOURCE=.\hasbroi.ico
# End Source File
# Begin Source File

SOURCE=.\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\Psx\mpl0.pal
# End Source File
# Begin Source File

SOURCE=.\Psx\mpl1.pal
# End Source File
# Begin Source File

SOURCE=.\Psx\mpl2.pal
# End Source File
# Begin Source File

SOURCE=.\Psx\mpl3.pal
# End Source File
# Begin Source File

SOURCE=.\Psx\mpl4.pal
# End Source File
# Begin Source File

SOURCE=.\Psx\mpl5.pal
# End Source File
# Begin Source File

SOURCE=.\Psx\mpl6.pal
# End Source File
# Begin Source File

SOURCE=.\Psx\mpl7.pal
# End Source File
# Begin Source File

SOURCE=.\Psx\pinkbaby.pal
# End Source File
# Begin Source File

SOURCE=X:\TeamSpirit\pcversion\reactive.txt
# End Source File
# Begin Source File

SOURCE=.\Psx\redbaby.pal
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\resource.rc

!IF  "$(CFG)" == "FroggerProject - Win32 Release"

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug"

!ELSEIF  "$(CFG)" == "FroggerProject - Win32 Debug with Bink Video"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Psx\Code\Saveicon.tim
# End Source File
# Begin Source File

SOURCE=X:\TeamSpirit\sonofbuglist.txt
# End Source File
# Begin Source File

SOURCE=.\todo.txt
# End Source File
# Begin Source File

SOURCE=.\Psx\yellowbaby.pal
# End Source File
# End Target
# End Project
