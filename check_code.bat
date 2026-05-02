@echo off
cmd /k "(
echo ================================================
echo Running Static Analysis via compile_commands.json
echo ================================================
echo.
echo Using your build configuration for accurate analysis...
echo.

if not exist "C:\Program Files\Cppcheck\cppcheck.exe" (
    echo ERROR: cppcheck not found!
    echo Please install from: http://cppcheck.sourceforge.net/
    echo.
    pause
    exit /b 1
)

if not exist "build/Debug/compile_commands.json" (
    echo ERROR: compile_commands.json not found!
    echo Please build your project first.
    echo.
    pause
    exit /b 1
)

:: Create cache folder if it doesn't exist
if not exist "cppcheck_cache" mkdir cppcheck_cache

:: Uncomment the next line for exhaustive analysis (slower but deeper)
:: set CHECK_LEVEL=--check-level=exhaustive

:: Leave this as default for normal analysis (faster)
set CHECK_LEVEL=

echo [1/4] Running cppcheck with compile_commands.json...
echo.

"C:\Program Files\Cppcheck\cppcheck.exe" ^
    --project=build/Debug/compile_commands.json ^
    --cppcheck-build-dir=cppcheck_cache ^
    %CHECK_LEVEL% ^
    --enable=all ^
    --library=posix ^
    --template=gcc ^
    --inline-suppr ^
    --inconclusive ^
    --suppressions-list=cppcheck_suppressions.txt ^
    --error-exitcode=1

if errorlevel 1 (
    echo.
    echo ================================================
    echo FAILED: Static analysis violations found
    echo ================================================
    echo.
    pause
    exit /b 1
)

echo.
echo [2/4] Generating XML report (with standard classifications)...
echo.

"C:\Program Files\Cppcheck\cppcheck.exe" ^
    --project=build/Debug/compile_commands.json ^
    --cppcheck-build-dir=cppcheck_cache ^
    %CHECK_LEVEL% ^
    --enable=all ^
    --library=posix ^
    --inline-suppr ^
    --inconclusive ^
    --suppressions-list=cppcheck_suppressions.txt ^
    --report-type=misra-c-2012 ^
    --report-type=cert-c-2016 ^
    --xml ^
    --xml-version=2 ^
    --output-file=cppcheck_report.xml

echo.
echo [3/4] Generating CLEAN HTML report (suppressed warnings omitted)...
echo.

if exist "C:\Users\camer\AppData\Local\Python\pythoncore-3.14-64\Scripts\cppcheck-htmlreport.exe" (
    "C:\Users\camer\AppData\Local\Python\pythoncore-3.14-64\Scripts\cppcheck-htmlreport.exe" ^
        --file=cppcheck_report.xml ^
        --report-dir=cppcheck_html_report ^
        --source-dir=.
    echo Clean HTML report generated in cppcheck_html_report\index.html
) else (
    echo cppcheck-htmlreport.exe not found. Skipping HTML report.
)

echo.
echo [4/4] Generating COMPLETE HTML report (includes all warnings for documentation)...
echo.

:: Generate complete report WITHOUT suppressions
"C:\Program Files\Cppcheck\cppcheck.exe" ^
    --project=build/Debug/compile_commands.json ^
    --cppcheck-build-dir=cppcheck_cache ^
    %CHECK_LEVEL% ^
    --enable=all ^
    --library=posix ^
    --inline-suppr ^
    --inconclusive ^
    --report-type=misra-c-2012 ^
    --report-type=cert-c-2016 ^
    --xml ^
    --xml-version=2 ^
    --output-file=cppcheck_report_COMPLETE.xml

if exist "C:\Users\camer\AppData\Local\Python\pythoncore-3.14-64\Scripts\cppcheck-htmlreport.exe" (
    "C:\Users\camer\AppData\Local\Python\pythoncore-3.14-64\Scripts\cppcheck-htmlreport.exe" ^
        --file=cppcheck_report_COMPLETE.xml ^
        --report-dir=cppcheck_html_report_COMPLETE ^
        --source-dir=.
    echo Complete HTML report generated in cppcheck_html_report_COMPLETE\index.html
) else (
    echo cppcheck-htmlreport.exe not found. Skipping complete HTML report.
)

:: Log suppressions for documentation
echo.
echo [Info] Documenting suppressions...
echo.

echo === SUPPRESSED WARNINGS === > suppressions_log.txt
echo Generated: %date% %time% >> suppressions_log.txt
echo. >> suppressions_log.txt
type cppcheck_suppressions.txt >> suppressions_log.txt
echo. >> suppressions_log.txt
echo === End of suppressed warnings === >> suppressions_log.txt
echo Suppressions logged to: suppressions_log.txt

echo.
echo ================================================
echo PASSED: No violations found in your code
echo ================================================
echo.
echo Reports generated:
echo   - Clean XML: cppcheck_report.xml
echo   - Clean HTML: cppcheck_html_report\index.html
echo   - Complete XML: cppcheck_report_COMPLETE.xml
echo   - Complete HTML: cppcheck_html_report_COMPLETE\index.html
echo   - Suppressions log: suppressions_log.txt
echo   - Cache folder: cppcheck_cache
echo.
pause
exit /b 0
)"