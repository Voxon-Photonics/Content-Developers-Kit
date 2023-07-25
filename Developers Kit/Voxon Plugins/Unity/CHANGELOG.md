# Version 1.1.1.5 (2023-02-23)
 
## Major Changes
### Features
* 2D / 3D toggle feature so you can hibernate the VX1
* Support for a Helix (circle) display
* New Example Scene, Showing 3D Models and VoxieText Script
* Embedded Emulation rotational controls press '[' ']' + Right Shift || Right Ctrl to rotate emulation display


### Bugs

* Improved stability - Start / Stop in the Unity Editor shouldn't crash   
Unity anymore (Crashes can still occur when using the inspector at times - working on this)

* Postscript Build Process now creates a VX.bat and copies supporting DLLs again


# Version 1.1.1.4 (2022-02-07)

## Major Changes
### Features
* Moved to new Camera system. Uses VXCamera prefab objects to maintain correct aspect ratio
* Updated to add support for VCB recording, and local lighting

### Bugs
* Fixed SkinnedMesh scaling. Required move to 2020.3 ([20947a](https://bitbucket.org/voxon-photonics/unitypackage/commit/20947ab3d1365fa4d60224685a2eabc62e16dc7e))

## Minor Changes
### Features
* Added Gamma, Density, DotSize and Normal Lighting Support (Including Example Scenes)
* Added Dynamic Mesh Deformation

### Bugs
* Fixed tinting for textured materials
* Prevent generation of empty compute buffers when vertex count is 0

# Version 1.1.0.3 (2021-03-05)

## Minor Changes
### Features
* Mesh and Texture Cache: Moved from C# Serialization to Structs packed in Byte arrays stored per Scene now ([f933c2](https://bitbucket.org/voxon-photonics/unitypackage/commit/f933c24020bcab8645083518282734d8114092f3))
* Animator using obj sequences ([f933c2](https://bitbucket.org/voxon-photonics/unitypackage/commit/f933c24020bcab8645083518282734d8114092f3))
* Added Performance Examples ([d8b479](https://bitbucket.org/voxon-photonics/unitypackage/commit/d8b4792b85ef3e2949b923838279dc971f26af93))
* Can now show VPS via front panel ([d8b479](https://bitbucket.org/voxon-photonics/unitypackage/commit/d8b4792b85ef3e2949b923838279dc971f26af93))

### Bugs
* Objects are now cached at the mesh level rather than the file level (was causing issues with FBX files) ([f2649f](https://bitbucket.org/voxon-photonics/unitypackage/commit/f2649f8ddb4443700699371801a1fc97bc50b5e4))

# Version 0 - 1.0.0.2 (2020-07-07)

## Features
* Added Windows Message Box support for improved exception reporting
* Updated to C#-Bridge-Interface version 1.0.7488.24763. First versioned dll release
* Added Touch Screen Menu creation
* Added Support For Emulator Camera rotation and distance
* Draw Bulk Voxels
* Added Colour Swap Mode to allow developers to swap colour palettes on the fly. Will extend into R/B, R/G, and B/G sets in future
* Added Dynamic Texture Support, Included example scene, VideoPlayer will now be detected by VXComponent
* Added support for DrawVoxels
* Exposed the Internal and External Radius functions
* Added arrow keys to keyboard enum
* Added Camera Navigator (Present in Example 11)
* Dynamic Object loading (Example 11)
* Initial Helix function
* New example scene for Voxels
* Added Space Nav support & example scripts
* Added level loading example
* Updated Unity Plugin to use C# bridge. System is now DLL agnostic
* Added new types; Box, Cube, Sphere, Voxel, Line
* Added Prebuilding of Mesh data
* Added handling for unhidden components
* Moved over to interfaces to allow for drawing
* Added black mat to blank objects (Deprecated)
* Removed CaptureVolume prefab; added _camera prefab
* Added Mesh Reimporter to fix some scale issues

## Deprecations
* Removed Experimental.PlayerLoop to make compatible with 2019.3
* Removed forced radius values (unnecessary)

## Bugs
* Fixed registry check being disabled
* VXVoxel fix. holding off replacing vxvoxel until impact assessment
* Fixed UV bug in Registered Mesh (Broke Animated Textures)
* Fix broken Colour Mode
* Finalised Colour Mixer and reenabled Dual Color mode after fix was applied to C#Bridge
* Caught and resolved a bug duplicating Textures
* Corrected detection of main textures for components
* Fixed the Randomised Voxel script
* Fixed non-uniform texture bug
* Corrected Numpad_Minux spelling mistake
* Fixed ComputeShader cleanup
* Fixed Active in Hierarchy bug, Fixed Name problem
* Fixed input hashcode incompatibility. Will now allow input files to work across machines
* Fixed lag due to copying array vs accessing members
* Fixed GameObject adding VXComponent to particle systems

## Tweak
* Moved Plugins to Voxon base folder
* Added help messages to some example scenes. Cleaned up materials
* Updated Input names to better reflect associated projects. Fixed Camera control for 11_Dynamic Loading and adjusted controls for 9_SpaceNav
* Cleaned up input naming system
* Updated Runtime to test for "VOXON_RUNTIME" path variable
* Path variables only check user environment now
* Moved Texture Reflection example script into 13_CameraTextures
* Moved package to Unity 2018.4.15f, 
* Slowed camera in 11_DynamicLoading, 
* Inverted VoxieCamTuner so it appear the world rotates
* Added Rainbow object to DynamicLoading (brighter)
* No longer detailing items to be skipped in the log
* Updated Camera Turner, now more closely mimics VoxieOs bindings and integrates bother SpaceNav and Keyboard
* Added SDK / DLL version functions
* Moved Voxon specific code into Voxon(...) namespaces. Unified naming conventions, implemented (almost) all clean up suggested by rider
* Removed old IRuntimePromise.dll, it's now part of the C# bridge
* Updated system to Unity 2019, handle the new VX interface
* Added MVC for Box, Cube, Sphere, and Particles object types
* Updated VX bridge. Fixed types to use new draw functions
* Moved functions to use point3d instead of floats for points passed to VX.dll
* Renamed IVXDrawable to IDrawable (it's already in the Voxon namespace)
* Cleaned up Warnings for non-use of variables, removed debug logs for creation / access of singletons
* Now handles Save when no Streaming Assets folder
* Moved 'VoxonPhotonics/Examples/Scenes' to 'Scenes/Voxon Examples' (searching for these will now be easier)
* Renamed VoxonUnity to VoxonPhotonics