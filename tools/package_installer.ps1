param(
    [string]$Version = '5.7.4',
    [switch]$ReimportLabviewData,
    [switch]$InstallCompiler
)

$ErrorActionPreference = 'Stop'

$repo = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$portableScript = Join-Path $PSScriptRoot 'package_portable.ps1'
$installerScript = Join-Path $repo 'installer\gucds.iss'
$distRoot = Join-Path $repo 'dist'
$setupName = "QL-IOT-App-$Version-win-x64-Setup.exe"
$setupPath = Join-Path $distRoot $setupName
$languageDir = Join-Path $repo 'build\installer-languages'
$chineseLanguageFile = Join-Path $languageDir 'ChineseSimplified.isl'
$chineseLanguageUrl = 'https://raw.githubusercontent.com/kira-96/Inno-Setup-Chinese-Simplified-Translation/6da09d23e14443d4cf8f07b1c5fd821bfe459788/ChineseSimplified.isl'
$chineseLanguageSha256 = '869E43E7C7B8D20C7E4397C8E98F7D1B7CF0528803ACDF019AD350143EC85469'

if ($Version -notmatch '^\d+\.\d+\.\d+(\.\d+)?$') {
    throw "Version must contain three or four numeric components: $Version"
}

function Find-InnoCompiler {
    $command = Get-Command 'ISCC.exe' -ErrorAction SilentlyContinue
    if ($command) {
        return $command.Source
    }

    $candidates = @(
        (Join-Path $env:LOCALAPPDATA 'Programs\Inno Setup 6\ISCC.exe'),
        'C:\Program Files (x86)\Inno Setup 6\ISCC.exe',
        'C:\Program Files\Inno Setup 6\ISCC.exe'
    )
    foreach ($candidate in $candidates) {
        if (Test-Path -LiteralPath $candidate) {
            return $candidate
        }
    }
    return $null
}

$iscc = Find-InnoCompiler
if (!$iscc -and $InstallCompiler) {
    $winget = Get-Command 'winget.exe' -ErrorAction SilentlyContinue
    if (!$winget) {
        throw 'winget.exe is unavailable. Install Inno Setup 6 manually, then rerun this script.'
    }
    & $winget.Source install `
        --id JRSoftware.InnoSetup `
        --exact `
        --scope user `
        --silent `
        --accept-package-agreements `
        --accept-source-agreements
    if ($LASTEXITCODE -ne 0) {
        throw "Inno Setup installation failed with exit code $LASTEXITCODE"
    }
    $iscc = Find-InnoCompiler
}
if (!$iscc) {
    throw 'Inno Setup 6 was not found. Rerun with -InstallCompiler, or install JRSoftware.InnoSetup with winget.'
}

New-Item -ItemType Directory -Force -Path $languageDir | Out-Null
$languageHash = if (Test-Path -LiteralPath $chineseLanguageFile) {
    (Get-FileHash -Algorithm SHA256 -LiteralPath $chineseLanguageFile).Hash
} else {
    $null
}
if ($languageHash -ne $chineseLanguageSha256) {
    Invoke-WebRequest -Uri $chineseLanguageUrl -OutFile $chineseLanguageFile -UseBasicParsing
    $languageHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $chineseLanguageFile).Hash
}
if ($languageHash -ne $chineseLanguageSha256) {
    throw "Simplified Chinese Inno Setup language file hash mismatch: $languageHash"
}

$portableArguments = @()
if ($ReimportLabviewData) {
    $portableArguments += '-ReimportLabviewData'
}
& $portableScript @portableArguments
if ($LASTEXITCODE -ne 0) {
    throw "Portable package build failed with exit code $LASTEXITCODE"
}

& $iscc "/DMyAppVersion=$Version" "/DChineseLanguageFile=$chineseLanguageFile" '/Qp' $installerScript
if ($LASTEXITCODE -ne 0) {
    throw "Inno Setup compiler failed with exit code $LASTEXITCODE"
}
if (!(Test-Path -LiteralPath $setupPath)) {
    throw "Installer output was not created: $setupPath"
}

$hash = Get-FileHash -Algorithm SHA256 -LiteralPath $setupPath
$hashPath = "$setupPath.sha256.txt"
"$($hash.Hash)  $setupName" | Set-Content -LiteralPath $hashPath -Encoding ASCII

Write-Output "installer ready: $setupPath"
Write-Output "sha256: $($hash.Hash)"
Write-Output "checksum file: $hashPath"
