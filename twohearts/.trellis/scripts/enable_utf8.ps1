$OutputEncoding = [System.Text.UTF8Encoding]::new($false)
[Console]::InputEncoding = [System.Text.UTF8Encoding]::new($false)
[Console]::OutputEncoding = [System.Text.UTF8Encoding]::new($false)
$env:PYTHONIOENCODING = "utf-8"
$env:PYTHONUTF8 = "1"

try {
    chcp 65001 | Out-Null
} catch {
}

Write-Host "已为当前 PowerShell 会话启用 UTF-8 控制台模式。"
Write-Host "PYTHONIOENCODING=utf-8"
Write-Host "PYTHONUTF8=1"
