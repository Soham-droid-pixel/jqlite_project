# demo.ps1 - Demonstration script for jqlite

Write-Host "=============================================" -ForegroundColor Cyan
Write-Host "   jqlite - JSON Query Tool Demo" -ForegroundColor Cyan
Write-Host "=============================================" -ForegroundColor Cyan
Write-Host ""

Write-Host "EXAMPLE 1: Get entire document" -ForegroundColor Yellow
.\jqlite.exe '.' test.json
Write-Host ""

Write-Host "EXAMPLE 2: Get name field" -ForegroundColor Yellow
.\jqlite.exe '.name' test.json
Write-Host ""

Write-Host "EXAMPLE 3: Get first post title" -ForegroundColor Yellow
.\jqlite.exe '.posts[0].title' test.json
Write-Host ""

Write-Host "EXAMPLE 4: Get second post likes" -ForegroundColor Yellow
.\jqlite.exe '.posts[1].likes' test.json
Write-Host ""

Write-Host "EXAMPLE 5: Get nested setting" -ForegroundColor Yellow
.\jqlite.exe '.settings.theme' test.json
Write-Host ""

Write-Host "EXAMPLE 6: Error handling" -ForegroundColor Yellow
.\jqlite.exe '.invalid' test.json
Write-Host ""

Write-Host "=============================================" -ForegroundColor Green
Write-Host "Demo Complete!" -ForegroundColor Green
Write-Host "=============================================" -ForegroundColor Green
