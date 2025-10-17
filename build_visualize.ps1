# build_visualize.ps1
# PowerShell build script for jqlite visualization mode

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Building jqlite Visualization Mode" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Check for required tools
$tools = @("win_bison", "win_flex", "gcc")
$missingTools = @()

foreach ($tool in $tools) {
    try {
        $null = Get-Command $tool -ErrorAction Stop
    } catch {
        $missingTools += $tool
    }
}

if ($missingTools.Count -gt 0) {
    Write-Host "ERROR: Missing required tools: $($missingTools -join ', ')" -ForegroundColor Red
    Write-Host "Run build.ps1 first to check tool availability" -ForegroundColor Yellow
    exit 1
}

# Step 1: Generate visualization lexer
Write-Host "[1/7] Generating visualization lexer..." -ForegroundColor Yellow
win_flex -P query_yy -o query_visualize.lex.c query_visualize.l
if ($LASTEXITCODE -ne 0) {
    Write-Host "Error: Failed to generate lexer" -ForegroundColor Red
    exit 1
}
Write-Host "Lexer generated" -ForegroundColor Green

# Step 2: Generate visualization parser
Write-Host "[2/7] Generating visualization parser..." -ForegroundColor Yellow
win_bison -d -p query_yy -o query_visualize.tab.c query_visualize.y
if ($LASTEXITCODE -ne 0) {
    Write-Host "Error: Failed to generate parser" -ForegroundColor Red
    exit 1
}
Write-Host "Parser generated" -ForegroundColor Green

# Step 3: Compile main_visualize.c
Write-Host "[3/7] Compiling main_visualize.c..." -ForegroundColor Yellow
gcc -c main_visualize.c -o main_visualize.o
if ($LASTEXITCODE -ne 0) {
    Write-Host "Error: Failed to compile main_visualize.c" -ForegroundColor Red
    exit 1
}
Write-Host "main_visualize.o created" -ForegroundColor Green

# Step 4: Compile engine_visualize.c
Write-Host "[4/7] Compiling engine_visualize.c..." -ForegroundColor Yellow
gcc -c engine_visualize.c -o engine_visualize.o
if ($LASTEXITCODE -ne 0) {
    Write-Host "Error: Failed to compile engine_visualize.c" -ForegroundColor Red
    exit 1
}
Write-Host "engine_visualize.o created" -ForegroundColor Green

# Step 5: Compile generated visualization parser
Write-Host "[5/7] Compiling query_visualize.tab.c..." -ForegroundColor Yellow
gcc -c query_visualize.tab.c -o query_visualize.tab.o
if ($LASTEXITCODE -ne 0) {
    Write-Host "Error: Failed to compile query_visualize.tab.c" -ForegroundColor Red
    exit 1
}
Write-Host "query_visualize.tab.o created" -ForegroundColor Green

# Step 6: Compile generated visualization lexer
Write-Host "[6/7] Compiling query_visualize.lex.c..." -ForegroundColor Yellow
gcc -c query_visualize.lex.c -o query_visualize.lex.o
if ($LASTEXITCODE -ne 0) {
    Write-Host "Error: Failed to compile query_visualize.lex.c" -ForegroundColor Red
    exit 1
}
Write-Host "query_visualize.lex.o created" -ForegroundColor Green

# Step 7: Link all objects (reuse normal JSON parser)
Write-Host "[7/7] Linking jqlite_viz.exe..." -ForegroundColor Yellow
gcc -o jqlite_viz.exe main_visualize.o engine_visualize.o `
    query_visualize.tab.o query_visualize.lex.o `
    json.tab.o json.lex.o
if ($LASTEXITCODE -ne 0) {
    Write-Host "Error: Failed to link jqlite_viz.exe" -ForegroundColor Red
    exit 1
}
Write-Host "jqlite_viz.exe created" -ForegroundColor Green

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "Build Complete!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Executable: jqlite_viz.exe" -ForegroundColor Cyan
Write-Host 'Test: .\jqlite_viz.exe --visualize ".posts" test.json' -ForegroundColor Cyan
Write-Host ""
