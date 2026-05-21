$OutputEncoding = [System.Text.UTF8Encoding]::new($false)
[Console]::InputEncoding = [System.Text.UTF8Encoding]::new($false)
[Console]::OutputEncoding = [System.Text.UTF8Encoding]::new($false)
$env:PYTHONIOENCODING = "utf-8"
$env:PYTHONUTF8 = "1"

try {
    chcp 65001 | Out-Null
} catch {
}

Write-Host "UTF-8 console mode enabled for this PowerShell session."
Write-Host "PYTHONIOENCODING=utf-8"
Write-Host "PYTHONUTF8=1"
