The files within this folder are for setting environment path and registry values for the Voxon Runtime.

File List: 

Add Voxon Unity Plugin Registry Keys.reg – Registry Keys for the Voxon Unity Plugin (assumes you have installed the Developers Kit to C:\Voxon)

Set Path and Registry.bat – Batch file for setting path variable and registry keys (assumes you have installed the Developers Kit to C:\Voxon)

SetVariables.ps1 – PowerShell script for setting path variable and registry keys (assumes you have installed the Developers Kit to C:\Voxon)
 

More Information:

1. Environment Path Settings

Adding the Voxon Runtime to the environment path allows VX applications to not need a local version of Voxiebox.dll (and other associated DLLs) to run applications. 
You can set this manually by typing in 'environment variables' in Windows Start and launching the control panel option. Click 'Edit' on the path variable and add 
the directory path to where the Voxon\System\Runtime folder is on your machine.

2. Registry Settings

The "Add Voxon Unity Plugin Registry Keys".reg are the registry keys needed for the Voxon Unity Plugin to work. These settings assume the SDK is installed under ‘C:\Voxon\’
If you have side loaded the Developers Kit to another folder. You can edit these values in RegEdit (Windows Start and type in ‘RegEdit’) the keys you need are located at 'Computer\HKEY_CURRENT_USER\Software\Voxon\Voxon'


