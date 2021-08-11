cls
@echo off
SETLOCAL EnableDelayedExpansion
echo Voxon Photonics Setup - add Voxon to path and to the registry 
echo.
echo This batch file will do the following:
echo.
echo 	1) Add Voxon Runtime directory to the User's Path variable 
echo 	2) Add Voxon Runtime directory to the system's registry. 
echo.
echo.  Press "Y" to Add Voxon Runtime to the path variable and system registry.
echo.  Press "Q" to abort setup and exit. 
choice /c:YQ 
IF ERRORLEVEL ==2 GOTO ABORT
IF ERRORLEVEL ==1 GOTO INSTALL
:INSTALL

set runtime_path=%~dp0%runtime\

set runtime_prompt=""
echo %PATH% > %TEMP%\Path.txt
echo [Runtime Path]
echo.
echo. 
FOR /F "tokens=* USEBACKQ" %%F IN (`findstr /R /C:".*%runtime_path%.*" "%TEMP%\Path.txt"`) DO (
	set runtime_prompt=%%F
)

if %runtime_prompt% EQU "" (
	CHOICE /m "Add Voxon Runtime directory to env path (%runtime_path%)"
	IF !ERRORLEVEL! EQU 1 (
		echo     Install Runtime
		endlocal
		SET "PATH=%runtime_path%;%PATH%"
		SETLOCAL EnableDelayedExpansion
	)
	IF !ERRORLEVEL! EQU 2 (
		echo     Skipping...
	)
) else (
	echo     Voxon Runtime directory already in env path. Skipping...
)

echo [Registry]

reg query HKCU\Software\Voxon\Voxon > nul 2> nul

if %ERRORLEVEL% EQU 0 goto NO_REG
if %ERRORLEVEL% EQU 1 goto INSTALL_REG  

GOTO QUIT

:NO_REG
echo     Voxon Runtime registry keys found. Skipping...
GOTO QUIT

:INSTALL_REG
CHOICE /m "Add Voxon paths to registry"
IF !ERRORLEVEL! EQU 1 (
	echo     Adding registry keys
	reg add HKCU\Software\Voxon\Voxon /v Path /t REG_SZ /d %runtime_path%
)
IF !ERRORLEVEL! EQU 2 (
	echo     Skipping...
)
GOTO QUIT

:ABORT
echo. Setup aborted. No changes to your system were made.

:QUIT
pause
