param(
    [Parameter(Mandatory = $true)]
    [string]$EngineRoot,

    [switch]$SetUserEnvironmentVariable
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$resolvedEngineRoot = (Resolve-Path $EngineRoot).Path
$buildBat = Join-Path $resolvedEngineRoot "Engine\Build\BatchFiles\Build.bat"

if (-not (Test-Path $buildBat)) {
    throw "Build.bat was not found under '$resolvedEngineRoot'. Please pass the Unreal Engine root folder, for example H:\UE_5.6."
}

$configDir = Join-Path $projectRoot "Saved\Unreal"
$configPath = Join-Path $configDir "EngineRoot.txt"

New-Item -ItemType Directory -Path $configDir -Force | Out-Null
Set-Content -Path $configPath -Value $resolvedEngineRoot -NoNewline

Write-Host ">> Saved workspace Unreal Engine path to $configPath"
Write-Host ">> Engine root: $resolvedEngineRoot"

if ($SetUserEnvironmentVariable) {
    [System.Environment]::SetEnvironmentVariable("UE_ENGINE_ROOT", $resolvedEngineRoot, "User")
    Write-Host ">> Updated user environment variable UE_ENGINE_ROOT"
    Write-Host ">> Restart VS Code terminals if they are already open."
}
