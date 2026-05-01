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
