cls
set thisPath=%cd%

@echo off
echo. VoxiePy -> A Python wrapper for Voxon Development
echo. 2023 - Matthew Vecchio for Voxon. 
echo.
echo. This batch script will install VoxiePy to your Python 3 library
echo. VoxiePy has been developed to work with Python 3.11 but its likely 
echo. to work on older versions
echo.
echo. * Please ensure you have Pyhton 3 installed prior to running this script
echo.
echo.
echo. Press 1 to Add / Install VoxiePy to your Python 3's library folder 
echo. Press 2 to Remove / Uninstall VoxiePy from your Python 3's library folder
echo. Press 3 or Q to Quit This batch script.
choice /c 123Q 
if errorlevel 4 Goto Exit
if errorlevel 3 Goto Exit
if errorlevel 2 Goto Uninstall
if errorlevel 1 Goto Install

:Uninstall
echo. VoxPy Removing

C:
cd %LocalAppData%\Programs\Python\Python3*\Lib
set installPath=%cd%
dir "vx*.py" "vox*.py"

Del %installPath%\VxScanCodes.py
Del %installPath%\VoxiePy.py 
Del %installPath%\VxPyDataTypes.py
Del %installPath%\VxPyTools.py
cd installPath
dir "vx*.py" "vox*.py"

echo. VoxiePy Removed from  Python 3's Lib folder
pause
goto Exit

:Install 
echo. VoxiePy Installing

C:
cd %LocalAppData%\Programs\Python\Python3*\Lib
set installPath=%cd%

Copy "%thisPath%\VxScanCodes.py" %installPath%
Copy "%thisPath%\VxPyDataTypes.py" %installPath%
Copy "%thisPath%\VxPyTools.py" %installPath%
Copy "%thisPath%\VoxiePy.py" %installPath%


cd installPath
dir "vx*.py"
echo. VxPy Installed into Python 3's Lib folder
pause
goto Exit


:Exit
echo. Quiting...
%thisPath:~0,2%
cd "%thisPath%"