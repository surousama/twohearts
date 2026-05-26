$utf8NoBom = [System.Text.UTF8Encoding]::new($false)

$OutputEncoding = $utf8NoBom
[Console]::InputEncoding = $utf8NoBom
[Console]::OutputEncoding = $utf8NoBom
$env:PYTHONIOENCODING = "utf-8"
$env:PYTHONUTF8 = "1"

try {
    chcp 65001 | Out-Null
} catch {
}

# PowerShell 5.1 keeps file-content cmdlets on their own default encoding.
# Switching only console/stdout encoding is not enough for UTF-8 files without BOM.
$encodingTargets = @(
    "Get-Content:Encoding",
    "Set-Content:Encoding",
    "Add-Content:Encoding",
    "Out-File:Encoding",
    "Export-Csv:Encoding",
    "Import-Csv:Encoding"
)

foreach ($target in $encodingTargets) {
    $global:PSDefaultParameterValues[$target] = "utf8"
}

Write-Host "UTF-8 console mode enabled for common PowerShell text workflows."
Write-Host "PYTHONIOENCODING=utf-8"
Write-Host "PYTHONUTF8=1"
Write-Host "Default Encoding for Get-Content/Set-Content/Out-File is now pinned to utf8."
Write-Host ""
Write-Host "Important: `powershell -File .\\.trellis\\scripts\\enable_utf8.ps1` only affects that child process."
Write-Host "To keep the settings in your current PowerShell session, run this inside the current shell:"
Write-Host "  . .\\.trellis\\scripts\\enable_utf8.ps1"
