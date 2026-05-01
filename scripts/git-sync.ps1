param(
    [Parameter(Position = 0)]
    [string]$Message
)

$ErrorActionPreference = "Stop"

if (-not $Message) {
    $Message = "chore: sync local changes"
}

$branch = (git branch --show-current).Trim()

if (-not $branch) {
    Write-Error "Unable to determine the current git branch."
}

git add -A

$hasChanges = git diff --cached --quiet
if ($LASTEXITCODE -eq 0) {
    Write-Host "No staged changes to commit."
    exit 0
}

git commit -m $Message
git push origin $branch
