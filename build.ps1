# build.ps1
# PowerShell build script for jqlite (works without make)
# This script manually compiles all components

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "jqlite - Manual Build Script" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Check for required tools
$tools = @{
    "win_bison" = "Bison parser generator"
    "win_flex" = "Flex lexer generator"
    "gcc" = "GCC C compiler"
}

$missingTools = @()

foreach ($tool in $tools.Keys) {
    Write-Host "Checking for $tool..." -NoNewline
    try {
        $null = Get-Command $tool -ErrorAction Stop
        Write-Host " OK" -ForegroundColor Green
    } catch {
        Write-Host " NOT FOUND" -ForegroundColor Red
        $missingTools += $tool
    }
}

if ($missingTools.Count -gt 0) {
    Write-Host ""
    Write-Host "ERROR: Missing required tools: $($missingTools -join ', ')" -ForegroundColor Red
    Write-Host ""
    Write-Host "To install on Windows with MSYS2:" -ForegroundColor Yellow
    Write-Host "  1. Open MSYS2 terminal" -ForegroundColor Yellow
    Write-Host "  2. Run: pacman -S flex bison make" -ForegroundColor Yellow
    Write-Host "  3. Add C:\msys64\usr\bin to your PATH" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Or download from:" -ForegroundColor Yellow
    Write-Host "  - Flex/Bison for Windows: https://github.com/lexxmark/winflexbison" -ForegroundColor Yellow
    exit 1
}

Write-Host ""
Write-Host "Step 1: Generating JSON parser..." -ForegroundColor Cyan
win_bison -d -p json_yy -o json.tab.c json.y
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Failed to generate JSON parser" -ForegroundColor Red
    exit 1
}
Write-Host "  Generated: json.tab.c, json.tab.h" -ForegroundColor Green

Write-Host ""
Write-Host "Step 2: Generating JSON lexer..." -ForegroundColor Cyan
win_flex -P json_yy -o json.lex.c json.l
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Failed to generate JSON lexer" -ForegroundColor Red
    exit 1
}
Write-Host "  Generated: json.lex.c" -ForegroundColor Green

Write-Host ""
Write-Host "Step 3: Generating Query parser..." -ForegroundColor Cyan
win_bison -d -p query_yy -o query.tab.c query.y
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Failed to generate Query parser" -ForegroundColor Red
    exit 1
}
Write-Host "  Generated: query.tab.c, query.tab.h" -ForegroundColor Green

Write-Host ""
Write-Host "Step 4: Generating Query lexer..." -ForegroundColor Cyan
win_flex -P query_yy -o query.lex.c query.l
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Failed to generate Query lexer" -ForegroundColor Red
    exit 1
}
Write-Host "  Generated: query.lex.c" -ForegroundColor Green

Write-Host ""
Write-Host "Step 5: Compiling C source files..." -ForegroundColor Cyan
$sources = @("main.c", "engine.c", "json.tab.c", "json.lex.c", "query.tab.c", "query.lex.c")
$objects = @()

foreach ($src in $sources) {
    $obj = $src -replace '\.c$', '.o'
    Write-Host "  Compiling $src..." -NoNewline
    gcc -Wall -g -c $src -o $obj
    if ($LASTEXITCODE -ne 0) {
        Write-Host " FAILED" -ForegroundColor Red
        exit 1
    }
    Write-Host " OK" -ForegroundColor Green
    $objects += $obj
}

Write-Host ""
Write-Host "Step 6: Linking executable..." -ForegroundColor Cyan
gcc -Wall -g -o jqlite.exe $objects
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Failed to link executable" -ForegroundColor Red
    exit 1
}
Write-Host "  Created: jqlite.exe" -ForegroundColor Green

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "BUILD SUCCESSFUL!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "To run examples:" -ForegroundColor Yellow
Write-Host "  .\jqlite.exe '.' test.json" -ForegroundColor White
Write-Host "  .\jqlite.exe '.name' test.json" -ForegroundColor White
Write-Host "  .\jqlite.exe '.posts[0].title' test.json" -ForegroundColor White
Write-Host ""
