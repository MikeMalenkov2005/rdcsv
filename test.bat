@echo off
setlocal enabledelayedexpansion

REM Build
call build.bat
if errorlevel 1 (
    echo Build failed
    exit /b 1
)

set total=0
set passed=0
set failed=0

for %%f in (test\t*.csv) do (
    set "csv=%%f"
    set "filename=%%~nf"
    set "N=!filename:t=!"
    set "expected_exit=test\expected\e!N!.txt"
    set "expected_output=test\expected\o!N!.txt"

    if not exist "!expected_exit!" (
        echo Missing expected exit file for test !N!, skipping
        goto :continue
    )
    if not exist "!expected_output!" (
        echo Missing expected output file for test !N!, skipping
        goto :continue
    )

    set "tempout=%TEMP%\rdcsv_test_!N!_!RANDOM!.tmp"

    REM Run the app, capture combined stdout+stderr
    rdcsv.exe "!csv!" > "!tempout!" 2>&1
    set exit_code=!errorlevel!

    REM Read expected exit code (strip spaces)
    set /p expected=<"!expected_exit!"
    
    if !exit_code! equ !expected! (
        REM Compare output (use findstr /v /x /c: to compare line by line, but simpler: fc)
        fc "!tempout!" "!expected_output!" > nul 2>&1
        if errorlevel 1 (
            echo Test !N! FAILED: output mismatch
            set /a failed+=1
        ) else (
            echo Test !N! PASSED
            set /a passed+=1
        )
    ) else (
        echo Test !N! FAILED: exit code mismatch: got !exit_code!, expected !expected!
        set /a failed+=1
    )

    del "!tempout!" 2>nul
    :continue
    set /a total+=1
)

echo Summary: %passed% passed, %failed% failed out of %total%
if %failed% neq 0 exit /b 1
exit /b 0

