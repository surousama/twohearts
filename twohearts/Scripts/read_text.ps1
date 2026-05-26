param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string]$Path,

    [int]$StartLine = 1,

    [int]$EndLine = 0,

    [int]$First = 0,

    [switch]$ShowEncoding
)

$utf8NoBom = [System.Text.UTF8Encoding]::new($false)
$OutputEncoding = $utf8NoBom
[Console]::InputEncoding = $utf8NoBom
[Console]::OutputEncoding = $utf8NoBom
$env:PYTHONIOENCODING = "utf-8"
$env:PYTHONUTF8 = "1"

function New-StrictEncoding($Name) {
    return [System.Text.Encoding]::GetEncoding(
        $Name,
        [System.Text.EncoderExceptionFallback]::new(),
        [System.Text.DecoderExceptionFallback]::new()
    )
}

function Decode-Bytes([byte[]]$Bytes) {
    if ($Bytes.Length -ge 3 -and $Bytes[0] -eq 0xEF -and $Bytes[1] -eq 0xBB -and $Bytes[2] -eq 0xBF) {
        return @{
            Name = "utf-8-bom"
            Text = [System.Text.UTF8Encoding]::new($true).GetString($Bytes, 3, $Bytes.Length - 3)
        }
    }

    if ($Bytes.Length -ge 4 -and $Bytes[0] -eq 0xFF -and $Bytes[1] -eq 0xFE -and $Bytes[2] -eq 0x00 -and $Bytes[3] -eq 0x00) {
        return @{
            Name = "utf-32-le"
            Text = [System.Text.Encoding]::UTF32.GetString($Bytes, 4, $Bytes.Length - 4)
        }
    }

    if ($Bytes.Length -ge 4 -and $Bytes[0] -eq 0x00 -and $Bytes[1] -eq 0x00 -and $Bytes[2] -eq 0xFE -and $Bytes[3] -eq 0xFF) {
        return @{
            Name = "utf-32-be"
            Text = [System.Text.Encoding]::GetEncoding(12001).GetString($Bytes, 4, $Bytes.Length - 4)
        }
    }

    if ($Bytes.Length -ge 2 -and $Bytes[0] -eq 0xFF -and $Bytes[1] -eq 0xFE) {
        return @{
            Name = "utf-16-le"
            Text = [System.Text.Encoding]::Unicode.GetString($Bytes, 2, $Bytes.Length - 2)
        }
    }

    if ($Bytes.Length -ge 2 -and $Bytes[0] -eq 0xFE -and $Bytes[1] -eq 0xFF) {
        return @{
            Name = "utf-16-be"
            Text = [System.Text.Encoding]::BigEndianUnicode.GetString($Bytes, 2, $Bytes.Length - 2)
        }
    }

    $fallbacks = @(
        @{ Name = "utf-8"; Encoding = [System.Text.UTF8Encoding]::new($false, $true) },
        @{ Name = "gb18030"; Encoding = (New-StrictEncoding "GB18030") },
        @{ Name = "gbk"; Encoding = (New-StrictEncoding 936) }
    )

    foreach ($candidate in $fallbacks) {
        try {
            return @{
                Name = $candidate.Name
                Text = $candidate.Encoding.GetString($Bytes)
            }
        } catch {
        }
    }

    return @{
        Name = "utf-8-replace"
        Text = [System.Text.UTF8Encoding]::new($false, $false).GetString($Bytes)
    }
}

try {
    $resolvedPath = Resolve-Path -LiteralPath $Path -ErrorAction Stop
    $bytes = [System.IO.File]::ReadAllBytes($resolvedPath.Path)
    $decoded = Decode-Bytes $bytes
    $lines = [regex]::Split($decoded.Text, "`r`n|`n|`r")

    if ($lines.Length -gt 0 -and $lines[-1] -eq "") {
        $lines = $lines[0..($lines.Length - 2)]
    }

    if ($First -gt 0) {
        $selected = $lines | Select-Object -First $First
    } else {
        $startIndex = [Math]::Max($StartLine - 1, 0)
        if ($EndLine -gt 0) {
            $count = [Math]::Max($EndLine - $StartLine + 1, 0)
            $selected = $lines | Select-Object -Skip $startIndex -First $count
        } else {
            $selected = $lines | Select-Object -Skip $startIndex
        }
    }

    if ($ShowEncoding) {
        [Console]::Error.WriteLine("encoding={0}" -f $decoded.Name)
    }

    foreach ($line in $selected) {
        [Console]::Out.WriteLine($line)
    }
} catch {
    [Console]::Error.WriteLine($_.Exception.Message)
    exit 1
}
