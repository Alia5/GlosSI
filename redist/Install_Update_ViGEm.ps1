
If (-NOT ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator"))
{   
$arguments = "& '" + $myinvocation.mycommand.definition + "'"
Start-Process powershell -Verb runAs -ArgumentList $arguments
Break
Return
}

if (get-module | Where-Object {$_.Name -eq "ViGEmManagementModule"}) 
{
 
    if (Get-ViGEmBusDevice)
    {
        Get-ViGEmBusDevice | Remove-ViGEmBusDevice
    }

} else {
    Register-PSRepository -Name nuget.vigem.org -SourceLocation https://nuget.vigem.org/ -InstallationPolicy Trusted

    Install-Module ViGEmManagementModule -Repository nuget.vigem.org
}


Install-ViGEmBusDeviceDriver

Add-ViGEmBusDevice