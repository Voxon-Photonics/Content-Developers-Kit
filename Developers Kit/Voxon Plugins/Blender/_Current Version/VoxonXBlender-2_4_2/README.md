# VoxonXBlender
A Blender add-on for creating recordings or displaying in real-time Blender projects on Voxon volumetric displays.

## Getting Started
### Requirements
 - Voxon Device with Voxon Runtime from May 3rd 2023 or later
   - A Windows computer running the VoxieOS Simulator may also be used
 - Blender version 3.0 or later (on any platform)
### Installing the Add-on
1. Download the add-on (link to be fixed)
2. In Blender, from the topbar go to Edit > Preferences
3. In the pop-up window, select "Add-ons" from the left menu.  In the top right corner, click "Install" and select the file you just downloaded.
4. An add-on called "Render: Voxon X Blender Add-On" should appear.  Select the checkbox next to the add-on name to enable it.
5. The add-on may not be up-to-date.  See "Updating the Add-on" in the next section.
## Functional Description
### Voxon Rendering Panel
The Voxon Rendering Panel can be found in Render Properties in Blender and contains most of the add-on's functionality.
### Viewbox
The viewbox is essentially a 3D camera for the volumetric display.  Only objects within the viewbox will be rendered.  The viewbox can be moved, rotated, scaled, and even animated.

The viewbox can be created from the Voxon Rendering Panel in two ways.  If "Auto-detect Viewbox" is selected when real-time rendering, the add-on will automatically create the correctly sized viewbox upon connecting to the Voxon Device.  Clicking "Generate Viewbox" will also create a viewbox, though you must make sure the aspect is set correctly.  The default aspect is correct for the VX1 and the VXR so it should only ever need to be changed if you are working with an experimental display.

When generating a viewbox when one already exists, the new viewbox will retain any animations applied to the old viewbox, even if the aspect has changed.
### Supported Materials
#### Supported Shaders
 - Principled BSDF (Blender's default)
 - Diffuse BSDF
#### Supported Base Colors
 - Solid color (both with and without using nodes)
 - Image Texture
    - Note: the add-on only sends image textures that are saved to image files, make sure to save your image texture files so the Voxon Device can reflect any changes you have made to the image textures
 - Color Attribute (vertex color)
#### Voxon Fillmode
 - Dot
 - Line (wireframe)
 - Surface
#### Other Limitations
 - If an object has multiple materials, only its first material will be rendered
 - The alpha color channel is currently not supported
### Real-Time Rendering
When real-time rendering, the add-on continuously sends data over the local network to a Voxon Device (theoretically it could also work over the Internet though it has never been tested).  To connect to a Voxon Device:
1. On the Voxon Device, open the Voxie Network app in Voxon Vertex
    - You can also use voxieplay
2. Type the IP address shown on the Voxon Device into the IP address field of the Voxon Rendering Panel.  If the shown port number is not 8080, add a ":" followed by the port number to the IP address you typed in.
    - You can change the device's default port number by changing it in voxiebox.ini
3. Select if you want to auto-detect viewbox (see "Viewbox" for more details) and if you want to render the viewbox, meaning that it will be outlined on the Voxon Device.
4. Click "Connect"
5. While the update should automatically send image textures and resend any image texture files that are changed, there is a chance that the image data will get lost and never recieved, especially when starting connection.  If this happens, click "Reload Image Textures" to resend them.

Real-time rendering is great for testing what objects will look like on the volumetric display.  However, rendering can become very slow when a Blender scene has lots of geometry.
### Recordings
The add-on can turn animated Blender scenes into recordings using the Voxon .REC format. These can be copied to the VX1 and played back with VoxieOS.  Advantages of this method include faster playback, more consistent frame rate and much smaller file size than animated .OBJ sequences.  They can also include audio.
#### Making Recordings
A recording is a folder containing a log file, that has debugging and appending data; a rec file, that has the actual recording data; and any needed external image and audio files.  Recordings can be created, replaced, or appened in the Voxon Rendering Panel.  In order to make a recording, the Blender scene must have a viewbox.  Animating at 15 fps is recommended because that is the speed the bottom and top of the VX1 are rendered at (the middle is rendered at 30 vps), but any fps can be used.

To create a recording, select the location for the recording to made and the name of the recording, then click "Make Recording".  If a folder already exists with the given name, the folder will instead be named name.XXX, where XXX is a number (the log and rec files will still be named with the given name).

To replace or append to a recording, select the recording's folder and then click "Make Recording".
#### Playing Recordings
On the Voxon Device open VoxieOS.  Navigate to the recording's folder.  In the recording's folder, select the rec file.

To play a recording with voxieplay, type voxieplay followed by the path of the recording's rec file into the command prompt.
### Updating the Add-on
1. From the topbar go to Edit > Preferences
2. In the pop-up window, select "Add-ons" from the left menu.  Find the VoxonXBlender add-on (using the search bar in the top right corner can help with this).
3. Select the arrow to drop down the updater menu.
4. If you enable "Auto-check for Update" you will be alerted in the Voxon Rendering panel when there is an update to the add-on
5. Click "Check now for voxon_x_blender update".
6. If an update is found, click "Update now to X.X.X" to update.  You can also trigger another check by clicking the refresh icon.
7. You change to any version of the add-on by clicking the "Install the master / old version" button and selecting your desired version.  You can also switch to the master branch of this repository, though be aware that this version may have more bugs than official releases.
8. If an updated add-on is not working correctly, clicking "Restore addon backup" will restore the previously loaded version of the add-on.
### Tips and Tricks
 - Disabling an object in renders will stop it from being rendered in real-time and recordings.  This does not apply to the viewbox
   - Currently, recording does not respect the animation of this value.  This will be fixed in the next update 
 - For solid colored objects, do not use nodes in their material for better performance
 - The log file of a recording will alert you if an object is not rendered.  It will also show the error traceback if a recording fails.
 - Black colored meshes are not rendered in real-time or recording since it will not be shown on the display anyways
 - The Voxon device renders based off of the object’s material, which is not necessarily how it will look in Blender depending on your display settings
## Troubleshooting
 - If your Voxon Device is giving you an error about a corrupted command, it is likely your Voxon Runtime needs to be updated
 - Make sure the add-on and voxieplay can send and receive data past the firewalls on their respective computers
 - When recording, check the log file to see the error
 - When replacing or appending to a recording, make sure the selected folder is actually a recording folder with a rec and log file.
 - Report any bugs on the "Issues" tab of this repository.  Please feel free to also request features th**ere as well!
## Code Description
**\_\_init__.py**: main file of the module containing Blender panels, Blender operators, real-time rendering code, viewbox code, and add-on information and fucntions

**addon_updater_ops.py** and **addon_updater.py**: code for the updater, adapted from [Patrick W. Crawford (TheDuckCow)](https://github.com/CGCookie/blender-addon-updater)

**bigpak.py**: a Python implementation of Ken Silverman's BigPak communication protocol used to send real-time rendering data

**voxieplay.py**: code to generate byte commands for voxieplay/Voxie Network

**voxierecording.py**: code to create/replace/append to recordings
