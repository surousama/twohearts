param(
    [ValidateSet("GenerateProjectFiles", "BuildEditor", "BuildGame", "CleanEditor", "CleanGame", "OpenEditor")]
    [string]$Action = "BuildEditor",

    [ValidateSet("DebugGame", "Debug", "Development", "Shipping")]
    [string]$Configuration = "Development",

    [ValidateSet("Win64")]
    [string]$Platform = "Win64"
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

function Resolve-EngineRoot {
    param(
        [string]$ProjectRoot,
        [string]$EngineAssociation
    )

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
            return (Resolve-Path $candidate).Path
        }
    }

    throw "Unable to locate an Unreal Engine installation for association '$EngineAssociation'."
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
$engineRoot = Resolve-EngineRoot -ProjectRoot $projectRoot -EngineAssociation $uprojectJson.EngineAssociation

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
