$tag = git describe --tags --always $(git rev-list --all --max-count=1)
if (-Not ($tag -match ".+\..+\..+\..+")) {
    $tag = "0.0.0." + $tag
}
Update-AppveyorBuild -Version $tag
