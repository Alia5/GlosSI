#stolen and adapted from: https://github.com/BrianLima/UWPHook/blob/master/UWPHook/Resources/GetAUMIDScript.ps1
$installedapps = Get-AppxPackage
$invalidNames = '*ms-resource*', '*DisplayName*'
$aumidList = @()

foreach ($app in $installedapps) {
    try {
        if (-not $app.IsFramework) {
            foreach ($id in (Get-AppxPackageManifest $app).package.applications.application.id) {
                $appx = Get-AppxPackageManifest $app;
                $name = $appx.Package.Properties.DisplayName;
				#filter out language packs and Sysinternals (adds ~30 entries, all non-game)
				if ($app.packagefamilyname -like "Microsoft.Language*" -or $name -like "Sysinternals*") { break; }
                if ($name -like '*DisplayName*' -or $name -like '*ms-resource*') {
                    $name = $appx.Package.Applications.Application.VisualElements.DisplayName;
                }							
                if ($name -like '*DisplayName*' -or $name -like '*ms-resource*') {
                    $name = $app.Name;
                }
				if ($name -like "MicrosoftWindows.*" -or $name -like "Microsoft.*" -or $name -like "Windows.*" -or $name -like "Windows Web*") { break; }

                $installDir = $app.InstallLocation;
				#filter out system apps and apps installed by Microsoft without a proper UI name (none of which are games)
                if ($installDir -like "*SystemApps*" -or $installDir -like "*Microsoft.Windows*") { break; }
				$logo = $app.InstallLocation + "\" + ($appx.Package.Applications.Application.VisualElements.Square150x150Logo | Select-Object -First 1);
				$assetpath = Split-Path -Path $logo -Parent
				$sp = (Split-Path -Path $logo -Leaf).Split(".")[0]
				$ext = ($appx.Package.Applications.Application.VisualElements.Square150x150Logo | Select-Object -First 1).Split(".")[-1]
				if (-Not ($logo | Test-Path -PathType Leaf)) { #if this app uses qualifiers to provide different scaling factors and does not have an unqualified name, select the largest of those icons
					$logo = Get-ChildItem -Path $($assetpath + '\' + $sp + '*.' + $ext) | Sort-Object -Property Length -Descending | Select-Object -First 1 -ExpandProperty FullName
				}

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

$aumidList | Sort-Object #sorts the array by treating it as single strings per app, thus the first field (app name) is what it sorts on
