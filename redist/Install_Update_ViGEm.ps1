
If (-NOT ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator"))
{   
$arguments = "& '" + $myinvocation.mycommand.definition + "'"
Start-Process powershell -Verb runAs -ArgumentList $arguments
Break
Return
}


#Remove multiple just in case... 
Get-ViGEmBusDevice | Remove-ViGEmBusDevice
Get-ViGEmBusDevice | Remove-ViGEmBusDevice
Get-ViGEmBusDevice | Remove-ViGEmBusDevice
Get-ViGEmBusDevice | Remove-ViGEmBusDevice


Register-PSRepository -Name nuget.vigem.org -SourceLocation https://nuget.vigem.org/ -InstallationPolicy Trusted -confirm:$false

Install-Module ViGEmManagementModule -Repository nuget.vigem.org

Install-ViGEmBusDeviceDriver

Add-ViGEmBusDevice