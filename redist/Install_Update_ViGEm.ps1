Register-PSRepository -Name nuget.vigem.org -SourceLocation https://nuget.vigem.org/ -InstallationPolicy Trusted -confirm:$false

Install-Module ViGEmManagementModule -Repository nuget.vigem.org

#Remove just in case... 
Get-ViGEmBusDevice | Remove-ViGEmBusDevice

Install-ViGEmBusDeviceDriver

Add-ViGEmBusDevice