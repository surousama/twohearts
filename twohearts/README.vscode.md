# VS Code Unreal Workflow

This project is ready to build from VS Code.

## Recommended extensions

- `ms-vscode.cpptools`
- `ms-vscode.cpptools-extension-pack`
- `ms-vscode.powershell`
- `ms-dotnettools.csharp`

## Common tasks

Open the command palette and run `Tasks: Run Task`, then choose one of these:

- `UE: Generate VS Code Files`
- `UE: Build Editor Development`
- `UE: Build Game Development`
- `UE: Clean Editor Development`
- `UE: Open Editor`
- `UE: Refresh And Build Editor`

`Ctrl+Shift+B` is mapped to `UE: Build Editor Development`.

## Script entry point

All tasks call:

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\Invoke-UE.ps1
```

Examples:

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\Invoke-UE.ps1 -Action GenerateProjectFiles
powershell -ExecutionPolicy Bypass -File .\Scripts\Invoke-UE.ps1 -Action BuildEditor -Configuration Development
powershell -ExecutionPolicy Bypass -File .\Scripts\Invoke-UE.ps1 -Action OpenEditor
```

The script auto-detects the `.uproject` file and resolves the matching Unreal Engine installation from the project metadata and solution references.

## Pin the local Unreal Engine path

If your home and office machines use different UE install paths, set one of these once per machine:

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\Set-UE-Path.ps1 -EngineRoot H:\UE_5.6
```

Optional: also write the user environment variable for VS Code workspace settings and terminals:

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\Set-UE-Path.ps1 -EngineRoot H:\UE_5.6 -SetUserEnvironmentVariable
```

Resolution order used by `Invoke-UE.ps1`:

1. `-EngineRoot` parameter
2. `UE_ENGINE_ROOT`
3. `UNREAL_ENGINE_ROOT`
4. `Saved\Unreal\EngineRoot.txt`
5. existing auto-detection

Every run now prints the UE path it is using, so you can see immediately which machine-local location was selected.
