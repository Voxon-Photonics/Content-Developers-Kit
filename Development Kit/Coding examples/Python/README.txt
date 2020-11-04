Voxon Python

Requirements:
Python 3.X 64-bit
voxiebox.dll

Usage:
python voxon_python.py


Notes:
This is an example program demonstrating the core loop of the voxon runtime operating in python. An example of how a developer may choose to structure a mesh is available in suzanne.py.

To extend this example, 
- read through voxiebox.txt (located in Development Kit / Documentation) for runtime functions and their parameters / return values
- define selected functions in Runtime::__init__ 
- call functions as self.vxDLL.<functionName> within core loop
- ensure any draw calls occur between voxie_frame_start and voxie_frame_end (This includes debug message calls)