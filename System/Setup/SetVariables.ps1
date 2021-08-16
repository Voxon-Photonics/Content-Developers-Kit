#Requires -RunAsAdministrator
function Abort {
	Write-Host "Setup aborted. No changes to your system were made."
	exit
}
function Test-RegistryValue {
	param (
		[parameter(Mandatory=$true)]
		[ValidateNotNullOrEmpty()]$Path,
	
		[parameter(Mandatory=$true)]
	 	[ValidateNotNullOrEmpty()]$Value
	)
	
	try {
		Get-ItemProperty -Path $Path | Select-Object -ExpandProperty $Value -ErrorAction Stop | Out-Null
	 	return $true
	}
	catch {
		return $false
	}	
}



Clear-Host
Write-Host "Voxon Photonics Setup - add Voxon to path and to the registry"
Write-Host ""
Write-Host "This batch file will do the following:"
Write-Host ""
Write-Host "	1) Add Voxon Runtime directory to the User's Path variable"
Write-Host "	2) Add Voxon Runtime directory to the system's registry. "
Write-Host ""
Write-Host "Press 'Y' to Add Voxon Runtime to the path variable and system registry."
Write-Host "Press 'Q' to abort setup and exit."

$Action = Read-Host '*'
if($Action -ne "Y"){
	Abort
}

Write-Host ""
Write-Host "[Runtime Path]"
$runtime_path=$PSScriptRoot+"\Runtime\"
$result = [Environment]::GetEnvironmentVariable('PATH', 'Machine') | Select-String -Pattern $runtime_path -SimpleMatch -Quiet

if($result -ne "True"){
	$Action=Read-Host "Add Voxon Runtime directory to env path ($runtime_path) [Y/N]"
	if($Action -ne "Y"){
		Write-Host "	Skipping..."
	} else {
		Write-Host "	Installing Runtime"

		$oldPath = [Environment]::GetEnvironmentVariable('PATH', 'Machine');
		[Environment]::SetEnvironmentVariable('PATH', "$runtime_path;$oldPath",'Machine');
		Write-Host "	Done."
	}
}
else {
	Write-Host "	Voxon Runtime directory already in env path. Skipping..."
}

Write-Host ""
Write-Host ""
Write-Host "[Registry]"

$result = Test-RegistryValue -Path 'HKCU:\SOFTWARE\Voxon\Voxon' -Value 'Path'
if($result -eq "True"){
	Write-Host "	Voxon Runtime registry keys found. Skipping..."
} else {
	$Action=Read-Host "Add Voxon paths to registry [Y/N]"
	if($Action -ne "Y"){
		Write-Host "	Skipping..."
	} else {
		Write-Host "	Adding registry keys"
		reg add HKCU\Software\Voxon\Voxon /V Path /T REG_SZ /D "$runtime_path"
		Write-Host "	Done."
	}
}
pause