#stolen and adapted from: https://github.com/BrianLima/UWPHook/blob/master/UWPHook/Resources/GetAUMIDScript.ps1
$installedapps = get-AppxPackage
$invalidNames = '*ms-resource*', '*DisplayName*'
$aumidList = @()

foreach ($app in $installedapps) {
    try {
        if (-not $app.IsFramework) {
            foreach ($id in (Get-AppxPackageManifest $app).package.applications.application.id) {
                $appx = Get-AppxPackageManifest $app;
                $name = $appx.Package.Properties.DisplayName;

                if ($name -like '*DisplayName*' -or $name -like '*ms-resource*') {
                    $name = $appx.Package.Applications.Application.VisualElements.DisplayName;
                }
                if ($name -like '*DisplayName*' -or $name -like '*ms-resource*') {
                    $name = $app.Name;
                }

                $installDir = $app.InstallLocation;
                $logo = $app.InstallLocation + "\" + $appx.Package.Applications.Application.VisualElements.Square150x150Logo;


                $aumidList += $name + "|" + $installDir + "|" + $logo + "|" + 
                $app.packagefamilyname + "!" + $id + ";"
            }
        }
    }
    catch {
        $ErrorMessage = $_.Exception.Message
        $FailedItem = $_.Exception.ItemName
    }
}

$aumidList;
