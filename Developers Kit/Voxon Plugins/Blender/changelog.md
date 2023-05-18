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

