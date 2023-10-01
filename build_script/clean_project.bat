@echo off
setlocal

echo cd ..\project
echo.

cd ..\project
for /r %%a in (.vs debug release x64) do (
    if exist "%%a" (
        echo Deleting "%%a"
        rd /s /q "%%a"
    )
)
echo complate

endlocal