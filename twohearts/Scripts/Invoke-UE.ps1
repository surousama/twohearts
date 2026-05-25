param(
    [ValidateSet("GenerateProjectFiles", "BuildEditor", "BuildGame", "CleanEditor", "CleanGame", "OpenEditor")]
    [string]$Action = "BuildEditor",

    [ValidateSet("DebugGame", "Debug", "Development", "Shipping")]
    [string]$Configuration = "Development",

    [ValidateSet("Win64")]
    [string]$Platform = "Win64",

    [string]$EngineRoot
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Get-ProjectRoot {
    return (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
}

function Get-UProjectPath {
    param(
        [string]$ProjectRoot
    )

    $uproject = Get-ChildItem -Path $ProjectRoot -Filter *.uproject -File | Select-Object -First 1
    if (-not $uproject) {
        throw "No .uproject file was found under '$ProjectRoot'."
    }

    return $uproject.FullName
}

function Resolve-ConfiguredEngineRoot {
    param(
        [string]$ProjectRoot,
        [string]$RequestedEngineRoot
    )

    $localConfigPath = Join-Path $ProjectRoot "Saved\Unreal\EngineRoot.txt"
    $configuredCandidates = [System.Collections.Generic.List[hashtable]]::new()

    if ($RequestedEngineRoot) {
        $configuredCandidates.Add(@{
            Source = "parameter -EngineRoot"
            Path = $RequestedEngineRoot
        })
    }

    if ($env:UE_ENGINE_ROOT) {
        $configuredCandidates.Add(@{
            Source = "environment variable UE_ENGINE_ROOT"
            Path = $env:UE_ENGINE_ROOT
        })
    }

    if ($env:UNREAL_ENGINE_ROOT) {
        $configuredCandidates.Add(@{
            Source = "environment variable UNREAL_ENGINE_ROOT"
            Path = $env:UNREAL_ENGINE_ROOT
        })
    }

    if (Test-Path $localConfigPath) {
        $localConfiguredPath = (Get-Content -Path $localConfigPath -Raw).Trim()
        if ($localConfiguredPath) {
            $configuredCandidates.Add(@{
                Source = "workspace config $localConfigPath"
                Path = $localConfiguredPath
            })
        }
    }

    foreach ($candidate in $configuredCandidates) {
        $candidatePath = $candidate.Path
        if (-not $candidatePath) {
            continue
        }

        $buildBat = Join-Path $candidatePath "Engine\Build\BatchFiles\Build.bat"
        if (Test-Path $buildBat) {
            return @{
                Path = (Resolve-Path $candidatePath).Path
                Source = $candidate.Source
            }
        }
    }

    return $null
}

function Resolve-EngineRoot {
    param(
        [string]$ProjectRoot,
        [string]$EngineAssociation,
        [string]$RequestedEngineRoot
    )

    $configuredEngine = Resolve-ConfiguredEngineRoot -ProjectRoot $ProjectRoot -RequestedEngineRoot $RequestedEngineRoot
    if ($configuredEngine) {
        return $configuredEngine
    }

    $candidates = [System.Collections.Generic.List[string]]::new()
    $driveRoot = [System.IO.Path]::GetPathRoot($ProjectRoot)

    if ($EngineAssociation) {
        $candidates.Add((Join-Path $driveRoot "UE_$EngineAssociation"))
        $candidates.Add((Join-Path $driveRoot "Epic Games\UE_$EngineAssociation"))
        $candidates.Add((Join-Path "C:\Program Files\Epic Games" "UE_$EngineAssociation"))
        $candidates.Add((Join-Path "D:\Epic Games" "UE_$EngineAssociation"))
        $candidates.Add((Join-Path "G:\" "UE_$EngineAssociation"))
    }

    $slnPath = Join-Path $ProjectRoot ((Split-Path $ProjectRoot -Leaf) + ".sln")
    if (Test-Path $slnPath) {
        $ubtLine = Select-String -Path $slnPath -Pattern 'UnrealBuildTool\\UnrealBuildTool\.csproj' | Select-Object -First 1
        if ($ubtLine) {
            $relativeCsproj = (($ubtLine.Line -split ',')[1]).Trim().Trim('"')
            $engineProgramsDir = Split-Path (Resolve-Path (Join-Path $ProjectRoot $relativeCsproj)).Path -Parent
            $engineRoot = Resolve-Path (Join-Path $engineProgramsDir "..\..\..\..\..")
            $candidates.Insert(0, $engineRoot.Path)
        }
    }

    $buildRegistry = Get-ItemProperty 'HKCU:\Software\Epic Games\Unreal Engine\Builds' -ErrorAction SilentlyContinue
    if ($buildRegistry) {
        foreach ($property in $buildRegistry.PSObject.Properties) {
            if ($property.Name -like 'PS*') {
                continue
            }

            if ($EngineAssociation -and $property.Name -eq $EngineAssociation) {
                $candidates.Insert(0, $property.Value)
            } else {
                $candidates.Add($property.Value)
            }
        }
    }

    foreach ($candidate in $candidates | Select-Object -Unique) {
        if (-not $candidate) {
            continue
        }

        $buildBat = Join-Path $candidate "Engine\Build\BatchFiles\Build.bat"
        if (Test-Path $buildBat) {
            return @{
                Path = (Resolve-Path $candidate).Path
                Source = "auto-detected from project metadata"
            }
        }
    }

    throw @"
Unable to locate an Unreal Engine installation for association '$EngineAssociation'.
You can fix this by using one of these options:
  1. pass -EngineRoot "X:\Path\To\UE_5.6"
  2. set the UE_ENGINE_ROOT environment variable
  3. create Saved\Unreal\EngineRoot.txt with the engine root path
"@
}

function Invoke-UeBuildBat {
    param(
        [string]$EngineRoot,
        [string[]]$Arguments
    )

    $buildBat = Join-Path $EngineRoot "Engine\Build\BatchFiles\Build.bat"
    if (-not (Test-Path $buildBat)) {
        throw "Build.bat was not found at '$buildBat'."
    }

    Write-Host ">> $buildBat $($Arguments -join ' ')"
    & $buildBat @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Unreal build command failed with exit code $LASTEXITCODE."
    }
}

$projectRoot = Get-ProjectRoot
$uprojectPath = Get-UProjectPath -ProjectRoot $projectRoot
$projectName = [System.IO.Path]::GetFileNameWithoutExtension($uprojectPath)
$uprojectJson = Get-Content $uprojectPath -Raw | ConvertFrom-Json
$engineInfo = Resolve-EngineRoot -ProjectRoot $projectRoot -EngineAssociation $uprojectJson.EngineAssociation -RequestedEngineRoot $EngineRoot
$engineRoot = $engineInfo.Path

Write-Host ">> Using Unreal Engine: $engineRoot"
Write-Host ">> Engine source: $($engineInfo.Source)"

switch ($Action) {
    "GenerateProjectFiles" {
        Invoke-UeBuildBat -EngineRoot $engineRoot -Arguments @(
            "-projectfiles",
            "-VSCode",
            "-project=$uprojectPath",
            "-game",
            "-engine"
        )
    }
    "BuildEditor" {
        Invoke-UeBuildBat -EngineRoot $engineRoot -Arguments @(
            "${projectName}Editor",
            $Platform,
            $Configuration,
            "-Project=$uprojectPath",
            "-WaitMutex",
            "-FromMsBuild"
        )
    }
    "BuildGame" {
        Invoke-UeBuildBat -EngineRoot $engineRoot -Arguments @(
            $projectName,
            $Platform,
            $Configuration,
            "-Project=$uprojectPath",
            "-WaitMutex",
            "-FromMsBuild"
        )
    }
    "CleanEditor" {
        Invoke-UeBuildBat -EngineRoot $engineRoot -Arguments @(
            "${projectName}Editor",
            $Platform,
            $Configuration,
            "-Project=$uprojectPath",
            "-Clean"
        )
    }
    "CleanGame" {
        Invoke-UeBuildBat -EngineRoot $engineRoot -Arguments @(
            $projectName,
            $Platform,
            $Configuration,
            "-Project=$uprojectPath",
            "-Clean"
        )
    }
    "OpenEditor" {
        $editorExe = Join-Path $engineRoot "Engine\Binaries\Win64\UnrealEditor.exe"
        if (-not (Test-Path $editorExe)) {
            throw "UnrealEditor.exe was not found at '$editorExe'."
        }

        Write-Host ">> Launching $editorExe `"$uprojectPath`""
        Start-Process -FilePath $editorExe -ArgumentList "`"$uprojectPath`"" | Out-Null
    }
}
