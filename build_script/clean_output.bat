@echo off
setlocal

echo cd ..\project
echo.

cd ..\
for /r %%a in (output) do (
    if exist "%%a" (
        echo Deleting "%%a"
        rd /s /q "%%a"
    )
)
echo complate

endlocal