@echo Build generic
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" ..\project\safe99_generic\safe99_generic\safe99_generic.vcxproj /t:build /p:configuration=debug /p:platform=x64
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" ..\project\safe99_generic\safe99_generic\safe99_generic.vcxproj /t:build /p:configuration=debug /p:platform=Win32

"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" ..\project\safe99_generic\safe99_generic\safe99_generic.vcxproj /t:build /p:configuration=release /p:platform=x64
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" ..\project\safe99_generic\safe99_generic\safe99_generic.vcxproj /t:build /p:configuration=release /p:platform=Win32