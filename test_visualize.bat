@echo off
REM ============================================
REM Test script for jqlite Visualization
REM ============================================

echo.
echo ╔════════════════════════════════════════════╗
echo ║   🧪 Testing jqlite Visualization Mode    ║
echo ╚════════════════════════════════════════════╝
echo.

REM Check if jqlite_viz.exe exists
if not exist "jqlite_viz.exe" (
    echo ❌ Error: jqlite_viz.exe not found!
    echo 💡 Run build_visualize.bat first
    exit /b 1
)

echo ✓ jqlite_viz.exe found
echo.

REM Test 1: Simple field access
echo ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
echo Test 1: Simple field access (.name)
echo ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
echo.
jqlite_viz.exe --visualize ".name" test.json
echo.

REM Test 2: Pipe operation
echo ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
echo Test 2: Pipe operation (.posts ^| [0])
echo ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
echo.
jqlite_viz.exe --visualize ".posts | [0]" test.json
echo.

REM Test 3: Select filter
echo ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
echo Test 3: Select filter (.posts ^| select(.likes ^> 50))
echo ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
echo.
jqlite_viz.exe --visualize ".posts | select(.likes > 50)" test.json
echo.

echo ╔════════════════════════════════════════════╗
echo ║   ✅ All Tests Complete!                   ║
echo ╚════════════════════════════════════════════╝
echo.
echo 💡 Check the JSON output above for:
echo    - "tokens": Array of lexer tokens
echo    - "parseSteps": Array of parser reductions
echo    - "executionTrace": Array of execution steps
echo    - "finalResult": Query result
echo.
