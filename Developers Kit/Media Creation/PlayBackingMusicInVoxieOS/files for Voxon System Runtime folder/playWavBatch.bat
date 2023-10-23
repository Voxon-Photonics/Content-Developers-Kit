REM set the filename of the scenes you want to check, these can be zipped animations too
REM put this batchfile in the PATH directory and have VoxieOS launch it to play sounds before and after a sequence

REM file to start the backing music...
IF "%~1"=="0_SCENE_START.zip" GOTO playbacking

REM any extra sounds you want to play during the scenes
REM IF "%~1"=="scene2.kv6" GOTO play


REM file to stop the backing music...
IF "%~1"=="ZZ_SCENE_END.zip" GOTO stopbacking
GOTO exit

:playbacking
start /min playwav C:\Voxon\System\Runtime\music.wav

GOTO exit


:play
REM the active directory is where the media is so use absolute paths or relative to where the media is
start /min playwav C:\Voxon\System\Runtime\soundtest.wav
GOTO exit

:stopbacking
REM to stop music when using playwav you'll need to kill the task
start /min taskkill /f /im playwav.exe
GOTO exit

:exit
REM end of program

