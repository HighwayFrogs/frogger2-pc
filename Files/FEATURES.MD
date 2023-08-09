# Feature List
Contained here is a full list of features included in the mod.  

## Enhancements

### High Resolution (& Widescreen)
The original game ran at a maximum resolution of 1024x768.  
Higher resolutions are now be supported, and with that includes a widescreen patch to avoid stretching.  

### Debug Mode
Many debugging capabilities present in the game code have been enabled, though they have been made to only activate when the user enables them via a "Debug Mode" checkbox.  
A few small exceptions exist like debug logging to a log file so that we can better diagnose crashes and problems.  
Information about debug mode features is available in the [debug readme](./DEBUG.MD).  

### Configuration Window
Often times when the game would fail to start properly, it would exit without any warning.  
This meant if an issue occurred, it was very hard to fix the game.  
This has been fixed by making it so the configuration window will ALWAYS display when starting the game.  

**Resolution List:**  
The resolution list often times would be empty without a clear reason in the configuration window.  
This is believed to be fixed, and if it ever occurs again will use the resolution from the .ini configuration file.  

**Mouse Hiding:**
If the game was launched fullscreen, but had an error that made it not launch successfully, when it returned to the configuration screen the mouse would not be shown.  
This has been fixed so the mouse will show again when this occurs.  

## Compatibility

### Crash (Camera Divide By Zero)
In certain situations, the game camera could cause crash due to a divide by zero.  
It's unknown how common this crash was in a retail game environment, but it was common in our builds, likely due to very high framerate.  
We believe we have fixed this crash.  


### 32-Bit Color Fixes
The game had several points where it was hardcoded to use 16-bit colors.  
Since then, 32-bit colors have become the standard.  
The code has been updated to support different pixel formats by automatically converting to the target format.  

**Situations Fixed:**  
 - Any full-screen image such as the loading screen or gallery images would cover only half of the screen and show incorrect colors.  
 
**Situations Ignored:**  
 - `pcmisc.cpp` has a "Design viewer" which is basically a slideshow viewer of level designs. It is unused and supporting it is pointless because we can just look at the .bmp files directly.  


## Audio

### CD Audio Playback
The game's music was originally played back from the CD.
This is still supported, but comes with some additional notes.  

**Feature - Filesystem Playback:**  
Support has been added to play music as .wav files from a `music` folder.  
The way this works is that the mod has a list of filenames corresponding to the music tracks.  
The list is shown below, but when it goes to play a song, it will get the filename for the song.  
If a file with that name exists in the `music\` folder, it will use the file, otherwise it will attempt to play audio from the CD if there is one.  
You can replace the .wav files with whatever music you want if you want to change the music.  
Instructions for setting this up are in the setup guide in [this document](./RELEASE.MD).  

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
```

**Bugfix - Pausing:**  
In modern Windows, the game would always play the Garden track when unpausing the game, instead of whatever track had been paused.  
This has been fixed.  

**Problem - Music Does Not Repeat (Also: End of Level Music Skipped):**  
Starting in Windows Vista, the feature that tells programs when the CD audio finished playing no longer works.  
This is not fixable in Frogger 2's code without major rewrites to use modern audio systems, the problem is in Windows code.  
These issues do not occur when music is played from the .wav files since they use a different system for audio playback.  
Something like DxWnd, inmm.dll, etc can fix this, but it is recommended to just use .wav playback instead.  

## Video

### Use Video Files from Hard Drive
Normally the game gets its video files from the CD, but a feature has been added to first try to get it from the game folder before trying the CD.  
The video files can be copied from a CD (or other source such as the tape backup) to a folder you create called `Video\` inside of the game folder.  
If videos do not play, error messages describing the issue should be included in the debug log file `frogger2.log` described in the debug document.  

## Misc
### Removing Windows Registry
The Windows Registry is used to store many pieces of game configuration data in the retail release.  
Unfortunately, this can cause many many issues when trying to run the game.  
We have moved all of this data outside of the registry into a .ini file, which can be easily edited in a text editor.  
By default, it will try to create frogger2.ini inside the folder with the game assets.
If this is in the `C:\Program Files (x86)\` install folder, Windows may automatically hide that it's actually putting the file in `%localappdata%\VirtualStore\Program Files (x86)\Hasbro Interactive\Frogger2\`.  
If it is unable to create the file there, it will create it in `%USERPROFILE%`, which is usually `C:\Users\{your_username}\`.  

### Portability
The original game looked in the registry to know where the game files were kept, except for video files and music which were obtained from the CD.  
This meant it could be quite difficult to run the game. Often times this didn't work properly or required the game to be run as admin for it to find the right registry values.  
It also made it difficult to run the game without installing it too, or if you wanted to dump your physical CD and still have music.  
We have made the game fully portable, so all you need is a single folder containing the game folders, and that's all which is necessary for the game to run.  

### Running Without Admin
Running as admin should no longer be required to play the game.  
There are some debugging features which may require running as admin to function properly, but we tried to take care of that as best we could.  
 
TODO: Unfinished:
 - Speedrun Timer
 - Audio Sliders
 - Images in windowed mode.
 - GPUs show up twice in config window.
 - FMV Freezes clicking outside the window in windowed mode.
 - Keyboard input while not in focus.
 - Weird DirectX repeat issue.