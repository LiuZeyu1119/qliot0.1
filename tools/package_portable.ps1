param(
    [switch]$ReimportLabviewData
)

$ErrorActionPreference = 'Stop'

$repo = Resolve-Path (Join-Path $PSScriptRoot '..')
$build = Join-Path $repo 'build\win-release'
$distRoot = Join-Path $repo 'dist'
$packageName = 'GUCDSQt-portable-win-x64'
$packageDir = Join-Path $distRoot $packageName
$zipPath = Join-Path $distRoot "$packageName.zip"

$qtBin = 'C:\Qt\6.8.3\mingw_64\bin'
$cmakeBin = 'C:\Qt\Tools\CMake_64\bin'
$ninjaBin = 'C:\Qt\Tools\Ninja'
$mingwBin = 'C:\Qt\Tools\mingw1310_64\bin'
$env:PATH = "$cmakeBin;$ninjaBin;$mingwBin;$qtBin;" + $env:PATH

& (Join-Path $qtBin 'qt-cmake.bat') -S $repo -B $build -G Ninja -DCMAKE_BUILD_TYPE=Release
if ($LASTEXITCODE -ne 0) {
    throw "qt-cmake failed with exit code $LASTEXITCODE"
}

& (Join-Path $ninjaBin 'ninja.exe') -C $build
if ($LASTEXITCODE -ne 0) {
    throw "ninja failed with exit code $LASTEXITCODE"
}

$labviewRoot = Join-Path $repo 'General Upper Computer Debugging Software5.5'
$dataDir = Join-Path $repo 'data'
$sqlitePath = Join-Path $dataDir 'gucds.sqlite'
$importer = Join-Path $build 'gucds_import_labview_data.exe'
New-Item -ItemType Directory -Force -Path $dataDir | Out-Null
if ($ReimportLabviewData -or !(Test-Path $sqlitePath)) {
    & $importer $labviewRoot $sqlitePath
    if ($LASTEXITCODE -ne 0) {
        throw "LabVIEW SQLite import failed with exit code $LASTEXITCODE"
    }
} else {
    Write-Output "Preserving existing SQLite data: $sqlitePath"
}

$env:QLIOT_SQLITE_PATH = $sqlitePath
& (Join-Path $cmakeBin 'ctest.exe') --test-dir $build --output-on-failure
if ($LASTEXITCODE -ne 0) {
    throw "ctest failed with exit code $LASTEXITCODE"
}

New-Item -ItemType Directory -Force -Path $distRoot | Out-Null
$resolvedDistRoot = (Resolve-Path $distRoot).Path
if (Test-Path $packageDir) {
    $resolvedPackageDir = (Resolve-Path $packageDir).Path
    if (!$resolvedPackageDir.StartsWith($resolvedDistRoot + [System.IO.Path]::DirectorySeparatorChar)) {
        throw "Refusing to remove unexpected package directory: $resolvedPackageDir"
    }
    Remove-Item -LiteralPath $resolvedPackageDir -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $packageDir | Out-Null

$appExe = Join-Path $build 'gucds_app.exe'
Copy-Item -LiteralPath $appExe -Destination $packageDir
New-Item -ItemType Directory -Force -Path (Join-Path $packageDir 'data') | Out-Null
Copy-Item -LiteralPath $sqlitePath -Destination (Join-Path $packageDir 'data\gucds.sqlite')

Copy-Item -LiteralPath $importer -Destination (Join-Path $packageDir 'gucds_import_labview_data.exe')

& (Join-Path $qtBin 'windeployqt.exe') `
    --release `
    --compiler-runtime `
    --translations zh_CN `
    --dir $packageDir `
    (Join-Path $packageDir 'gucds_app.exe')
if ($LASTEXITCODE -ne 0) {
    throw "windeployqt failed with exit code $LASTEXITCODE"
}

$readme = @(
    'GUCDS Qt Portable Package'
    ''
    'Start:'
    '  Double-click gucds_app.exe'
    ''
    'Language:'
    '  Use Settings -> Language to switch between Chinese and English.'
    ''
    'Data:'
    '  data\gucds.sqlite contains device, product, calibration, and bus records.'
    ''
    'Runtime:'
    '  Windows x86_64. Qt does not need to be installed on the target computer.'
    ''
    'Build kit:'
    '  C:\Qt\6.8.3\mingw_64'
)
$readme | Set-Content -LiteralPath (Join-Path $packageDir 'README-PORTABLE.txt') -Encoding UTF8

$requiredFiles = @(
    'gucds_app.exe',
    'gucds_import_labview_data.exe',
    'Qt6Core.dll',
    'Qt6Gui.dll',
    'Qt6Widgets.dll',
    'Qt6Sql.dll',
    'platforms\qwindows.dll',
    'sqldrivers\qsqlite.dll',
    'translations\qt_zh_CN.qm',
    'data\gucds.sqlite',
    'libgcc_s_seh-1.dll',
    'libstdc++-6.dll',
    'libwinpthread-1.dll'
)

foreach ($file in $requiredFiles) {
    $path = Join-Path $packageDir $file
    if (!(Test-Path $path)) {
        throw "Missing portable dependency: $file"
    }
}

$oldPath = $env:PATH
try {
    $env:PATH = "$env:SystemRoot\System32;$env:SystemRoot"
    $portableExe = Join-Path $packageDir 'gucds_app.exe'
    $process = Start-Process -FilePath $portableExe -WorkingDirectory $packageDir -PassThru -WindowStyle Hidden
    Start-Sleep -Seconds 3
    $process.Refresh()
    if ($process.HasExited) {
        throw "portable gucds_app exited early with code $($process.ExitCode)"
    }
    Stop-Process -Id $process.Id -Force
} finally {
    $env:PATH = $oldPath
}

if (Test-Path $zipPath) {
    Remove-Item -LiteralPath $zipPath -Force
}
Compress-Archive -LiteralPath $packageDir -DestinationPath $zipPath -Force
$zipHash = Get-FileHash -Algorithm SHA256 -LiteralPath $zipPath
$zipHashPath = "$zipPath.sha256.txt"
"$($zipHash.Hash)  $([System.IO.Path]::GetFileName($zipPath))" |
    Set-Content -LiteralPath $zipHashPath -Encoding ASCII

Write-Output "portable package ready: $packageDir"
Write-Output "portable zip ready: $zipPath"
Write-Output "portable zip sha256: $($zipHash.Hash)"
