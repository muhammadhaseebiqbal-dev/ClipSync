# PowerShell Installation Script for Ecosystem Windows Client
# Run this from an Administrator PowerShell terminal

$appName = "EcosystemClipboard"
$exeName = "main.exe"
$installDir = "$env:LOCALAPPDATA\$appName"
$exePath = "$installDir\$exeName"
$repoUrl = "https://raw.githubusercontent.com/YOUR_GITHUB_USERNAME/EcosystemClipboard/main/$exeName" # UPDATE THIS URL LATER

Write-Host "Creating installation directory: $installDir" -ForegroundColor Cyan
if (-not (Test-Path $installDir)) {
    New-Item -ItemType Directory -Path $installDir | Out-Null
}

Write-Host "Downloading background client..." -ForegroundColor Cyan
# Invoke-WebRequest -Uri $repoUrl -OutFile $exePath
# FOR LOCAL TESTING: Let's simply copy it instead of downloading since we haven't published to GitHub yet.
Copy-Item ".\WindowsClient\main.exe" -Destination $exePath -Force

Write-Host "Configuring Windows Auto-Startup..." -ForegroundColor Cyan
$registryPath = "HKCU:\Software\Microsoft\Windows\CurrentVersion\Run"
Set-ItemProperty -Path $registryPath -Name $appName -Value "`"$exePath`""

Write-Host "Starting Ecosystem Clipboard Service..." -ForegroundColor Green
Start-Process -FilePath $exePath -WindowStyle Hidden

Write-Host "--------------------------------------------------------" -ForegroundColor Green
Write-Host "SUCCESS!" -ForegroundColor Green
Write-Host "The client is now running invisibly in the background." -ForegroundColor White
Write-Host "It will automatically start whenever you turn on your PC." -ForegroundColor White
Write-Host "--------------------------------------------------------" -ForegroundColor Green
