![Frogger 2 Logo](/_repo/Frogger2Logo.png)
# Frogger 2 Enhanced (PC Mod)
Frogger 2 Enhanced is a Frogger 2 mod which improves the PC port to resolve bugs, fix compatibility issues, and add enhancements to the Frogger 2 PC port.

## Core Features
 - Easy installation  
 - Playing music from .wav files. (Instead of just CD)  
 - Playing videos from files. (Instead of just CD)  
 - Optionally enabling debug features such as the in-game editor.  
 - Fix compatibility problems that make running the game difficult.

## Setup Guide
The following guide is here to describe how to setup the Frogger 2 enhanced mod from scratch.  
It assumes you have a Frogger 2 CD or CD image to obtain files from, but it is possible to substitute this if you have some other method of getting the files such as from the tape backup.  
You are assumed to know what you're doing if you go this route, but support is provided by the [Highway Frogs](https://highwayfrogs.net/) community by joining the discord server.  

### 1) Install Frogger 2.
Run the CD setup at `\Frogger2\SETUP.EXE` as admin, or try `\Autorun.exe`.  
Go through the install process and install wherever you like, but remember the install folder. Usually it's `C:\Program Files (x86)\Hasbro Interactive\Frogger2\`.    
The installer is only important in the sense that it will put most of the files in the install folder. If you have the files from somewhere else, that will work too.  
From this point on, you could either use the install folder, or create a new folder somewhere else that you copy all of the files into. We will call this the "game folder" going forward.  

### 2) Install The Mod (Required)  
Replace `Frogger2.exe` in your install folder with the enhanced/modded `Frogger2.exe` or `FroggerProject.exe` file.  

### 3) Setup videos to play from files instead of CD (Recommended)  
Create a folder called `video` in the game folder.  
From the CD, copy all of the files in `\Video\` into the new video folder you've made.  
If you're getting the video files from the tape backup instead, they're at `D:\Frogger\Frogger2_Source_Delivery\Fmv\Pc\`.  

### 4) Setup music to play from files instead of CD (Recommended)  
Create a folder called `music` in the game folder.  

The way this works is that the mod has a list of filenames corresponding to the music tracks.  
The list is shown below, but when it goes to play a song, it will get the filename for the song.  
If a file with that name exists in the `music\` folder, it will use the file, otherwise it will attempt to play audio from the CD if there is one.  
To rip songs from a physical CD, either foobar2000 or PowerISO is recommended. For a virtual CD image (.bin/.cue), PowerISO is recommended.  
If using a virtual CD image, if you don't see the CD audio when opening it in PowerISO, most likely the image didn't include the music.  
Once you get the audio files, update their file names to match the list below.  
Alternatively, these files can be obtained from the development archives either from the `[2000-09-08] Final Audio` CD or from `D:\Frogger\Frogger2_Source_Delivery\CD Audio\Pc\` in the tape backup.  
You can replace the .wav files with whatever music you want if you want to change the music.  

**Music File List:**  
```
01 Garden.wav
02 Ancients.wav
03 Space.wav
04 Subterranean.wav
05 Laboratory.wav
06 Halloween.wav
07 retro.wav
08 Title.wav
09 level complete.wav
10 gameover.wav
11 EOL.wav
These file names are the official file names we got from the development archives.  
```

### 5) Setup In-game Editor Textures (Optional)  
The in-game editor icons will be gray cubes if this step is skipped. If you are not using the debug in-game editor, feel free to skip this.  
The editor bmp files should be included with the download of the enhancement mod, but they are also available in many of the PC prototypes or `\\BLITZONE\DATA\TeamSpirit\pcversion\Textures\Editor` in the tape backup.  
In your game folder, there should be a folder called `textures`. Inside this folder, a new folder called `Editor` should be created. Copy paste the .bmp files into this folder.  

### 6) Done
The game should now be possible to run just by running `Frogger2.exe` in your game folder.  
Support is provided by the [Highway Frogs](https://highwayfrogs.net/) community by joining the discord server.  

## Known Issues
**CD Audio Playback:**  
Music playback from CD cannot fully be fixed due to a Windows problem. The specific issues include music not repeating and end of level music not playing.  
The recommendation is to use .wav playback instead, as detailed in the setup guide, because playback from .wav files does not have this issue.  