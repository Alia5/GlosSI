$tag = git describe --tags --always
if (-Not ($tag -match ".+\..+\..+\..+")) {
    $tag = "0.0.0." + $tag
}
$commatag = $tag -replace "\.",","
$commatag = $commatag -replace "-","0"
$commatag = $commatag -replace "[A-z]","0"
((Get-Content -path ./Resource.rc -Raw) -replace "FILEVERSION .*,.*,.*,.*", ("FILEVERSION " + $commatag)) | Set-Content -Path ./Resource.rc
((Get-Content -path ./Resource.rc -Raw) -replace "PRODUCTVERSION .*,.*,.*,.*", ("PRODUCTVERSION " + $commatag)) | Set-Content -Path ./Resource.rc
((Get-Content -path ./Resource.rc -Raw) -replace '"FileVersion", ".*"', ('"FileVersion", "' + $tag + '"')) | Set-Content -Path ./Resource.rc
((Get-Content -path ./Resource.rc -Raw) -replace '"ProductVersion", ".*"', ('"ProductVersion", "' + $tag + '"')) | Set-Content -Path ./Resource.rc