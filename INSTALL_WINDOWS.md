# INSTALLATION GUIDE FOR WINDOWS

## Option 1: Quick Install with Chocolatey (Recommended)

If you have Chocolatey installed:
```powershell
choco install winflexbison3
```

Then run:
```powershell
.\build.ps1
```

## Option 2: Download WinFlexBison

1. Download from: https://github.com/lexxmark/winflexbison/releases
2. Extract the ZIP file
3. Add the extracted folder to your PATH, OR
4. Copy `win_bison.exe` and `win_flex.exe` to your project folder and rename them:
   - Rename `win_bison.exe` to `bison.exe`
   - Rename `win_flex.exe` to `flex.exe`

Then run:
```powershell
.\build.ps1
```

## Option 3: Use MSYS2

1. Install MSYS2 from: https://www.msys2.org/
2. Open MSYS2 terminal
3. Run: `pacman -S flex bison make`
4. Add `C:\msys64\usr\bin` to your Windows PATH
5. Restart PowerShell
6. Run: `.\build.ps1` or `make`

## Option 4: Quick Compile (Without Flex/Bison)

If you just want to demonstrate the C code quickly, I can provide
a pre-compiled version or use the alternative simple build below.

## Verify Installation

Run these commands to verify:
```powershell
gcc --version
bison --version
flex --version
```

All three should work before attempting to build.
