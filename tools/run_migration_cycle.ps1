$ErrorActionPreference = 'Stop'

$repo = Resolve-Path (Join-Path $PSScriptRoot '..')
$workspace = Resolve-Path (Join-Path $repo '..')
$build = Join-Path $workspace 'qt-rewrite-build'

$env:PATH = 'C:\Qt\Tools\CMake_64\bin;C:\Qt\Tools\Ninja;C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.8.3\mingw_64\bin;' + $env:PATH

& 'C:\Qt\6.8.3\mingw_64\bin\qt-cmake.bat' -S $repo -B $build -G Ninja
if ($LASTEXITCODE -ne 0) {
    throw "qt-cmake failed with exit code $LASTEXITCODE"
}

& 'C:\Qt\Tools\Ninja\ninja.exe' -C $build
if ($LASTEXITCODE -ne 0) {
    throw "ninja failed with exit code $LASTEXITCODE"
}

$labviewRoot = Join-Path $workspace 'General Upper Computer Debugging Software5.5'
$dataDir = Join-Path $repo 'data'
$sqlitePath = Join-Path $dataDir 'gucds.sqlite'
$importer = Join-Path $build 'gucds_import_labview_data.exe'
New-Item -ItemType Directory -Force -Path $dataDir | Out-Null
& $importer $labviewRoot $sqlitePath
if ($LASTEXITCODE -ne 0) {
    throw "LabVIEW SQLite import failed with exit code $LASTEXITCODE"
}
$env:QLIOT_SQLITE_PATH = $sqlitePath

& 'C:\Qt\Tools\CMake_64\bin\ctest.exe' --test-dir $build --output-on-failure
if ($LASTEXITCODE -ne 0) {
    throw "ctest failed with exit code $LASTEXITCODE"
}

$exe = Join-Path $build 'gucds_app.exe'
$process = Start-Process -FilePath $exe -PassThru -WindowStyle Hidden
Start-Sleep -Seconds 3
$process.Refresh()
if ($process.HasExited) {
    throw "gucds_app exited early with code $($process.ExitCode)"
}

Stop-Process -Id $process.Id -Force
Write-Output 'migration cycle passed'
