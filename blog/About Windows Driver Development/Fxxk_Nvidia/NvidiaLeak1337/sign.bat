@echo off

cd /d "%~dp0"

if "%~1"=="" goto help
goto sign

:help
echo Usage: sign.bat filename
echo Note: Original file will be overwritten by the signed one, no backup.
goto :end

:sign
echo Sign File %1

signtool.exe sign /v ^
    /ac cert.cer ^
    /f current_cert.pfx ^
    /p nv1d1aRules ^
    "%~1"

:end