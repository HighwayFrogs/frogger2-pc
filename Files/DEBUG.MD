# Debug Features
Frogger 2 had many debug features included.  
The enhanced mod enables these debug features for you if you enable the "Debug Mode" checkbox.  
Enabling debug mode will unlock all levels and characters automatically. All the other debug features need to be toggled manually.  

## Debug Logs
Even if the "Debug Mode" checkbox is unchecked, debug logs are created for the game to help diagnose any issues that might arise.  
The location the game wants to make these files is at `C:\frogger2.log` and `C:\mdebug.log`, however this will only occur if the program is run as admin or run on something old like Windows XP.  
Instead, the files will likely be created at `%localappdata%\VirtualStore\`, which is where modern Windows will redirect the files to go.  
When issues arise, looking inside of these files with any ol' text editor (such as Notepad) may include helpful warnings to guide troubleshooting.  

## Load .Bmps Option
This checkbox in the config window will make the game read textures from folders containing .bmp files placed in `Textures\` instead of .FLA files.  
You can get the texture folders from `D:\Frogger\Frogger2_Source_Delivery\Data\Pc\Textures\` or `\\BLITZONE\DATA\TeamSpirit\pcversion\Textures\` in the end of project tape backup.  
IF YOU DO NOT COPY ANY TEXTURES, THE GAME WILL LIKELY JUST CRASH. You can look at the `frogger2.log` file either at `C:\frogger2.log` or `%localappdata%\VirtualStore\frogger2.log` to find a log of what files it failed to load.  
This feature works regardless of if the "Debug Mode" checkbox is pressed.  

## Debug Keys:
F1: Increase Camera Height  
F2: Decrease Camera Height  
F3: Zoom Camera In (If LCtrl is pressed, Rotate Camera Left)  
F4: Zoom Camera Out (If LCtrl is pressed, Rotate Camera Right)  
F5: Toggle Debug Console    
F6: Toggle Timer Debug  
F7: Toggle Texture List  
F8: Toggle Displaying Tile ID  
F9: Take Screenshot (Requires command-line argument to enable)  
F10: Open In-game Editor  
F11: Toggle Sound Debug  
F12: Exit Game  
NUMPAD-7: Increase Fog  
NUMPAD-9: Decrease Fog  
NUMLOCK: Write texture information to a file in `C:\`.  

**Requires Controller:**  
These debug keybinds seem to require a game controller be inserted and configured for player 1, even if they are not possible to perform on the controller.  
R: Teleport to baby frog  
NUMPAD-4: Rotate Camera Left  
NUMPAD-6: Rotate Camera Right  

**Debug Console:**  
The debug console is much less useful than it sounds. It has these commands:
```
cls: Clear the console.
exit: Close the game.
set: View or set the value of a variable.
showobjects: Show all of the actors currently loaded.
```

**Texture List:**  
This is a display of currently loaded textures and if they are referenced.  
Pressing the up & down arrow keys will move through the texture list.  

**Sound Player:**  
This menu appears broken, it's supposed to render text in a different color than it's doing.  
Pressing up / down will change the active sound, though it doesn't show which sound is active.  
ENTER: Play sound.  

## In-game Editor
The in-game editor was used to create .dat files, aka entity placements, but has quite the number of features available.  
Use the arrow keys to move around, and right-click with the mouse to change the camera angle.  
Each feaure available with the mouse can be hovered to see what it is, but text coloring may make it hard to read.  
Press 'H' for a list of keybinds.  

**NOTE:** In order to make the editor icons display properly, you need to add the `Editor\` folder into `Textures\`.  
This is included in the release of this mod, but is also available in the end of project tape backup as well as prototype game builds.  

## Command-Line Arguments
The following arguments can be provided when starting the game.
```
R: Enable Record Keying
P: Enable Record Playing
A: Toggle Audio (Default: true)
D: Toggle Demo Playback (Default: false)
K: Toggle Debug Mode (Default: false)
S: Set Sort Mode to MA_SORTBACKFRONT.
I: Toggle Screenshot Mode (Default: false)
E: Toggle Memory Debug Messages (Default: false)
N: Start Network Game
```
Arguments can be input into the game in either the format `-K -D` or `-KD`.  