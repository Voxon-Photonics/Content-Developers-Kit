cls
@echo off
SETLOCAL EnableDelayedExpansion
echo ***************************************************************************
echo.         Voxon Photonics - 'Path and Registry Setup'   16/8/2021
echo ***************************************************************************
echo.
echo  Note: This setup asssumes you have installed The Developers Kit to 
echo  C:\VOXON. If you have the Developers Kit installed in another folder 
echo  please see the 'readme.txt' file in '.. \ System \ Setup'
echo. 
echo.
echo  This batch file will do the following:
echo.
echo 	1) Add Voxon Runtime (C:\VOXON\SYSTEM\RUNTIME) directory to the User's
echo        Path variable 
echo.
echo 	2) Add Voxon Runtime (C:\VOXON\SYSTEM\RUNTIME) directory to the system's 
echo.       registry. 
echo.
echo.  Press "Y" to Add Voxon Runtime to the path variable and system registry.
echo.  Press "Q" to abort setup and exit. 
echo.
echo  This batch file will launch a new Windows Powershell Window to execute you 
echo  may need to launch this batch file 'As Adminstrator' for it to work. 
echo.
choice /c:YQ
IF ERRORLEVEL ==2 GOTO ABORT
IF ERRORLEVEL ==1 GOTO INSTALL
:INSTALL

powershell -ExecutionPolicy Bypass -file "%~dp0%SetVariables.ps1"
echo. 
GOTO QUIT

:ABORT
echo. 
echo. Setup aborted. No changes to your system were made.
echo. 
echo. 
GOTO QUIT


:QUIT
Pause