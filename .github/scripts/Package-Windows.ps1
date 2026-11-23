#Requires -Version 7.2
[CmdletBinding()]
param(
    [ValidateSet('x64')]
    [string] $Target = 'x64',
    [ValidateSet('Debug', 'RelWithDebInfo', 'Release', 'MinSizeRel')]
    [string] $Configuration = 'RelWithDebInfo'
)

$ErrorActionPreference = 'Stop'
$ProjectRoot = (Resolve-Path "$PSScriptRoot/../..").Path
$ProductName = (Get-Content "$ProjectRoot/buildspec.json" -Raw | ConvertFrom-Json).name
$ReleaseDir = Join-Path $ProjectRoot 'release'
$Archive = Join-Path $ReleaseDir "$ProductName-windows-$Target.zip"
$Files = @(
    "$ReleaseDir/$Configuration/$ProductName/bin/64bit/$ProductName.dll"
    "$ProjectRoot/LICENSE"
    "$ProjectRoot/INSTALL.md"
    "$ProjectRoot/README.md"
    "$ProjectRoot/CHANGELOG.md"
    "$ProjectRoot/docs/RELEASE-NOTES.md"
)
foreach ($File in $Files) {
    if ((Get-Item -LiteralPath $File).Length -eq 0) {
        throw "Empty package input: $File"
    }
}

$TemporaryArchive = Join-Path $ReleaseDir "$([guid]::NewGuid()).zip"
try {
    Compress-Archive -LiteralPath $Files -DestinationPath $TemporaryArchive -CompressionLevel Optimal
    [IO.File]::Move($TemporaryArchive, $Archive, $true)
    Write-Output "Created $Archive"
} finally {
    if (Test-Path -LiteralPath $TemporaryArchive) { Remove-Item -LiteralPath $TemporaryArchive }
}
