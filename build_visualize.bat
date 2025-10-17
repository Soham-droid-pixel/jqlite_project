@echo off
REM ============================================
REM Build script for jqlite Visualization Mode
REM ============================================

echo.
echo ╔════════════════════════════════════════════╗
echo ║   🔬 Building jqlite Visualization Mode   ║
echo ╚════════════════════════════════════════════╝
echo.

REM Step 1: Generate visualization lexer
echo [1/7] Generating visualization lexer (query_visualize.l)...
flex -P query_yy -o query_visualize.lex.c query_visualize.l
if %ERRORLEVEL% NEQ 0 (
    echo ❌ Error: Failed to generate lexer
    exit /b 1
)
echo ✓ Lexer generated

REM Step 2: Generate visualization parser
echo [2/7] Generating visualization parser (query_visualize.y)...
bison -d -p query_yy -o query_visualize.tab.c query_visualize.y
if %ERRORLEVEL% NEQ 0 (
    echo ❌ Error: Failed to generate parser
    exit /b 1
)
echo ✓ Parser generated

REM Step 3: Compile main_visualize.c
echo [3/7] Compiling main_visualize.c...
gcc -c main_visualize.c -o main_visualize.o
if %ERRORLEVEL% NEQ 0 (
    echo ❌ Error: Failed to compile main_visualize.c
    exit /b 1
)
echo ✓ main_visualize.o created

REM Step 4: Compile engine_visualize.c
echo [4/7] Compiling engine_visualize.c...
gcc -c engine_visualize.c -o engine_visualize.o
if %ERRORLEVEL% NEQ 0 (
    echo ❌ Error: Failed to compile engine_visualize.c
    exit /b 1
)
echo ✓ engine_visualize.o created

REM Step 5: Compile generated visualization parser
echo [5/7] Compiling query_visualize.tab.c...
gcc -c query_visualize.tab.c -o query_visualize.tab.o
if %ERRORLEVEL% NEQ 0 (
    echo ❌ Error: Failed to compile query_visualize.tab.c
    exit /b 1
)
echo ✓ query_visualize.tab.o created

REM Step 6: Compile generated visualization lexer
echo [6/7] Compiling query_visualize.lex.c...
gcc -c query_visualize.lex.c -o query_visualize.lex.o
if %ERRORLEVEL% NEQ 0 (
    echo ❌ Error: Failed to compile query_visualize.lex.c
    exit /b 1
)
echo ✓ query_visualize.lex.o created

REM Step 7: Link all objects (reuse normal JSON parser)
echo [7/7] Linking jqlite_viz.exe...
gcc -o jqlite_viz.exe main_visualize.o engine_visualize.o ^
    query_visualize.tab.o query_visualize.lex.o ^
    json.tab.o json.lex.o
if %ERRORLEVEL% NEQ 0 (
    echo ❌ Error: Failed to link jqlite_viz.exe
    exit /b 1
)
echo ✓ jqlite_viz.exe created

echo.
echo ╔════════════════════════════════════════════╗
echo ║   ✅ Build Complete!                       ║
echo ╚════════════════════════════════════════════╝
echo.
echo 📦 Executable: jqlite_viz.exe
echo 🔬 Test: jqlite_viz.exe --visualize ".posts" test.json
echo.
