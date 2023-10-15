@echo Build math
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" ..\project\safe99_file_system\safe99_file_system\safe99_file_system.vcxproj /t:build /p:configuration=debug /p:platform=x64
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" ..\project\safe99_file_system\safe99_file_system\safe99_file_system.vcxproj /t:build /p:configuration=debug /p:platform=Win32

"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" ..\project\safe99_file_system\safe99_file_system\safe99_file_system.vcxproj /t:build /p:configuration=release /p:platform=x64
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" ..\project\safe99_file_system\safe99_file_system\safe99_file_system.vcxproj /t:build /p:configuration=release /p:platform=Win32