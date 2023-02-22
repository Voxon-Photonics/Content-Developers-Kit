cls
set thisPath=%cd%
@echo off
echo. VxPy -> A Python wrapper for Voxon Development
echo. 2023 - Matthew Vecchio for Voxon. 
echo.
echo. This batch script will install VxPy to your Python 3 library
echo. VxPy has been developed to work with Python 3.11 but its likely 
echo. to work on older versions
echo.
echo. * Please ensure you have Pyhton 3 installed prior to running this script
echo.
echo.
echo. Press 1 to Add / Install VxPy to your Python 3's library folder 
echo. Press 2 to Remove / Uninstall VxPy from your Python 3's library folder
echo. Press 3 or Q to Quit This batch script.
choice /c 123Q 
if errorlevel 4 Goto Exit
if errorlevel 3 Goto Exit
if errorlevel 2 Goto Uninstall
if errorlevel 1 Goto Install

:Uninstall
echo. VxPy Removing

C:
cd %LocalAppData%\Programs\Python\Python3*\Lib
set installPath=%cd%
dir "vx*.py"

Del %installPath%\VxPyConnect.py
Del  %installPath%\VxPy.py 
Del %installPath%\VxPyDataTypes.py
Del %installPath%\VxPyTools.py
cd installPath
dir "vx*.py"

echo. VxPy Removed from  Python 3's Lib folder
pause
goto Exit

:Install 
echo. VxPy Installing

C:
cd %LocalAppData%\Programs\Python\Python3*\Lib
set installPath=%cd%

Copy "%thisPath%\VxPyConnect.py" %installPath%
Copy "%thisPath%\VxPyDataTypes.py" %installPath%
Copy "%thisPath%\VxPyTools.py" %installPath%
Copy "%thisPath%\VxPy.py" %installPath%


cd installPath
dir "vx*.py"
echo. VxPy Installed into Python 3's Lib folder
pause
goto Exit


:Exit
echo. Quiting...
cd %thisPath:~0,2%
cd %thisPath%