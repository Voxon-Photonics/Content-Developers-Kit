# Voxon x Blender Add-on 
## By Ryan Silverman (rsil426@gmail.com)


### Version 1.0   
 
  * Original Ben Weatherall code solution using TCP
* Initial Release  
* 
### Version 2.0   

* Changed communication protocol from TCP to UDP/BigPak  
* 
### Version 2.1   

* Added fill mode support
* Added viewbox support to act as a camera that can be moved, rotated and 
 zoomed in and out  
 
### Version 2.2   

* Massive performance improvements from faster float to half conversion (uses external Python numpy module),  crc32c (use external Python crc32c module), and xor-ing  
* 
### Version 2.3  

NOTE: Requires VoxieBox.dll 3rd May 2023 and later 
* Vertex coloring support
* Image texturing support
 
**Note on rendering textures:** Image textures are only re-sent when the image file is saved because sending raw image buffer data would take up too much bandwidth. Once voxieplay is sent a texture file, it stores it in cache until voxieplay is exited.
 
**Note on rendering textures:** Image textures are only re-sent when the image file is saved because sending raw image buffer data would take up too much bandwidth. Once voxieplay is sent a texture file, it stores it in cache until voxieplay is exited.


### Version 2.3.2  

* Addition of bounding spheres to determine if a mesh is within the viewbox to render.
* Fixed bug with sending large meshes running into an overflow error. 
* Fixed bug where BigPak would error out after using the add-on for a long time

### Version 2.4.1  

#### Bug Fixes

* The add-on now reloads all of its files when being added, meaning Blender does not have to be restarted every time after updating the add-on
* Fixed a bug with bounding spheres algorithm (used in real-time rendering to detect if an object is potentially in the viewbox)
* Fixed WinError 10035 and 10054 bugs
* Made errno 65 error message more descriptive
* Black meshes no longer sent/recorded
* Stopped Blender file from trying to connect immediately after being opened if it was still connected when the file was last closed
* Corrected mistakes with viewbox regeneration
* Fixed problem that can occur with animated decimated meshes

#### Major Features

* Create recordings in Blender (.rec files)
* Recordings can be created, replaced, and appended to
* Same material support as real-time rendering
* Uses bounding box algorithm instead of bounding spheres (slower but significantly better at getting rid of out-of-view objects)
* Audio support
* Generates .log file to help with debugging and warn user when an object isn’t rendered
* “Reload Image Texture” button for real-time rendering
* Support for having multiple Blender scenes
* Only objects in the current scene are rendered
* Viewbox no longer needs to be named ‘VIEWBOX’
Built-in updater

#### Quality of Life Improvements

* Saves IP address across all files
* Avoids sending duplicate images in real-time rendering
* Add-on now shows version and author in Blender
* Cleans up data when disconnecting to save file space

### Version 2.4.2  

#### Bug Fixes

* Can now animate if object is rendered
* Fixed issue that would cause recordings with image textures or vertex coloring to fail
* Changed some of the text in the panels to make more sense

#### Features

* Recording now shows progress report with amount of frames rendered and an estimate of how much time is left
* Warning in Voxon Rendering Panel when a recording fails to make it more obvious