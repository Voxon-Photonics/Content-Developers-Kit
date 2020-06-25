:Menu
cls
@echo off
echo.               Voxon Voxatron Cart Launcher		
echo.              Curated by Voxon 14th June 2019
echo.       -= Press nominated key to launch a title =-
echo.
echo.       1 . 2-Player Banana             -   Tadashi
echo.       2 . Astronaut                   -   Kling
echo.       3 . Crossy Myrtle               -   Ramen Monster
echo.       4 . Cube Fusion                 -   D 
echo.       5 . Diamond For Miss Ola        -   Digital Monkey
echo.       6 . Enchant 1 : The Play        -   Kling
echo.       7 . Enchant 2 : Heart's Dungeon -   Kling
echo.       8 . Horde                       -   Kling
echo.       9 . Journey To The East         -   Kling
echo.       A . Katerinas Puzzle            -   CatsFurever
echo.       B . King's Quest                -   Mouser and Digital Monkey
echo.       C . The Lamia's Adventure       -   ChoasKing2
echo.       D . Longshot                    -   Chet-Rippo
echo.       E . Planet Of Mystery : Prelude -   D
echo.       F . Planet Of Mystery : 1       -   D
echo.       G . R(e)Xventure                -   Gacha
echo.       H . Snail Adventure             -   JerkStore
echo.       I . Stratogame I                -   Jauq
echo.       J . The Lamia's Adventure       -   ChoasKing2
echo.       K . Tinychase                   -   Kling
echo.       L . Valley of Shades            -   Cyzada
echo.       M . Voxacastle 1                -   Zmo
echo.       N . Valley of Shades            -   Cyzada
echo.       O . Voxajam 2019                -   enargy
echo.       P . VoxPaint                    -   Jauq
echo.       V . Voxatron w/ rotate_controls -   ZEP
echo.       --------------------------------------------
echo.       Q . Quit 
echo.



CHOICE /C 123456789ABCDEFGHIJKLMNOPVQ  /M "Please make a choice"
IF ERRORLEVEL 25 goto exit
IF ERRORLEVEL 24 vox.exe -rotate_controls 216 
IF ERRORLEVEL 23 vox.exe -rotate_controls 216 -vx_run carts\VoxyPaint.png
IF ERRORLEVEL 22 vox.exe -rotate_controls 216 -vx_run carts\voxajam2019-2.vx.png
IF ERRORLEVEL 21 vox.exe -rotate_controls 216 -vx_run carts\voxacastle1.png
IF ERRORLEVEL 20 vox.exe -rotate_controls 216 -vx_run carts\valleyofShades.png
IF ERRORLEVEL 19 vox.exe -rotate_controls 216 -vx_run carts\tinychase.png
IF ERRORLEVEL 18 vox.exe -rotate_controls 216 -vx_run carts\Stratogame.png
IF ERRORLEVEL 17 vox.exe -rotate_controls 216 -vx_run carts\snailAdventure.png
IF ERRORLEVEL 16 vox.exe -rotate_controls 216 -vx_run carts\Rexventure.png
IF ERRORLEVEL 15 vox.exe -rotate_controls 216 -vx_run carts\planetofMystery1.png
IF ERRORLEVEL 14 vox.exe -rotate_controls 216 -vx_run carts\planetofMystery0.png
IF ERRORLEVEL 13 vox.exe -rotate_controls 216 -vx_run carts\longshot.png
IF ERRORLEVEL 12 vox.exe -rotate_controls 216 -vx_run carts\LamiaAdventure.png
IF ERRORLEVEL 11 vox.exe -rotate_controls 216 -vx_run carts\kingsquest.png
IF ERRORLEVEL 10 vox.exe -rotate_controls 216 -vx_run carts\KaterinasPuzzle.png
IF ERRORLEVEL 9 vox.exe -rotate_controls 216 -vx_run carts\journey.png
IF ERRORLEVEL 8 vox.exe -rotate_controls 216 -vx_run carts\horde.png
IF ERRORLEVEL 7 vox.exe -rotate_controls 216 -vx_run carts\enhant2.png
IF ERRORLEVEL 6 vox.exe -rotate_controls 216 -vx_run carts\enhant.png
IF ERRORLEVEL 5 vox.exe -rotate_controls 216 -vx_run carts\diamond2d.png
IF ERRORLEVEL 4 vox.exe -rotate_controls 216 -vx_run carts\cube.png
IF ERRORLEVEL 3 vox.exe -rotate_controls 216 -vx_run carts\crossymyrtle.png
IF ERRORLEVEL 2 vox.exe -rotate_controls 216 -vx_run carts\astronaut.png
IF ERRORLEVEL 1 vox.exe -rotate_controls 216 -vx_run carts\bananas.png

goto Menu

:exit
echo. Ok. See you!