
If (-NOT ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator"))
{   
$arguments = "& '" + $myinvocation.mycommand.definition + "'"
Start-Process powershell -Verb runAs -ArgumentList $arguments
Break
Return
}

if (!(Get-Module -Listavailable -Name "ViGEmManagementModule"))
{
    Register-PSRepository -Name nuget.vigem.org -SourceLocation https://nuget.vigem.org/ -InstallationPolicy Trusted
    Install-Module ViGEmManagementModule -Repository nuget.vigem.org
}

Get-ViGEmBusDevice | Remove-ViGEmBusDevice
Add-ViGEmBusDevice
Install-ViGEmBusDeviceDriver