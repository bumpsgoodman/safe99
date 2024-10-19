@echo off
setlocal

cd ..\Project
for /r %%a in (.vs debug release x64) do (
    if exist "%%a" (
        rd /s /q "%%a"
	echo Deleted folder - %%a
    )
)

echo.
echo Complate

endlocal