# build_firmware.ps1 —— 激活 ESP-IDF 5.5.3 并跑 firmware 门禁等价步骤。
$ErrorActionPreference = "Stop"
$Repo = "C:\xiaomimimo\.mimo-sessions\2026\10\18\https-gitee-com-folotoy-ai-passport"
Set-Location $Repo

# Activate ESP-IDF in this process
& "C:\Users\FY\esp\esp-idf-v5.5.3\export.ps1" | Out-Null
Write-Host "IDF_PATH=$env:IDF_PATH"
& idf.py --version

$BuildDir = Join-Path $env:TEMP ("bagua-fw-" + [Guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Path $BuildDir | Out-Null
Write-Host "BuildDir=$BuildDir"

$env:SDKCONFIG_DEFAULTS = Join-Path $Repo "sdkconfig.defaults"
& idf.py -B $BuildDir -D "SDKCONFIG=$BuildDir\sdkconfig" build
if ($LASTEXITCODE -ne 0) { throw "idf.py build failed" }

& idf.py -B $BuildDir merge-bin -o (Join-Path $BuildDir "FoloToy-AI-Passport-full.bin")
if ($LASTEXITCODE -ne 0) { throw "merge-bin failed" }

& python tools/verify_firmware.py $BuildDir
if ($LASTEXITCODE -ne 0) { throw "verify_firmware failed" }

& python tools/archive_firmware.py create $BuildDir --archive-root (Join-Path $Repo "build\firmware")
if ($LASTEXITCODE -ne 0) { throw "archive_firmware failed" }

New-Item -ItemType Directory -Force -Path (Join-Path $Repo "build") | Out-Null
Copy-Item (Join-Path $BuildDir "FoloToy-AI-Passport-full.bin") (Join-Path $Repo "build\FoloToy-AI-Passport-full.bin") -Force
Write-Host "Firmware build: PASS"
Write-Host "full.bin => $Repo\build\FoloToy-AI-Passport-full.bin"
