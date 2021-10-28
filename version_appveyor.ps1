$tag = git describe --tags --always
if (-Not ($tag -match ".+\..+\..+\..+")) {
    $tag = "0.0.0." + $tag
}
Update-AppveyorBuild -Version $tag
