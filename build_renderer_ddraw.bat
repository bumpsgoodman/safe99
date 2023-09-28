@echo Build Runtime_x64
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" project\safe99_renderer_ddraw\safe99_renderer_ddraw\safe99_renderer_ddraw.vcxproj /t:rebuild /p:configuration=debug /p:platform=x64
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" project\safe99_renderer_ddraw\safe99_renderer_ddraw\safe99_renderer_ddraw.vcxproj /t:rebuild /p:configuration=debug /p:platform=Win32

"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" project\safe99_renderer_ddraw\safe99_renderer_ddraw\safe99_renderer_ddraw.vcxproj /t:rebuild /p:configuration=release /p:platform=x64
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" project\safe99_renderer_ddraw\safe99_renderer_ddraw\safe99_renderer_ddraw.vcxproj /t:rebuild /p:configuration=release /p:platform=Win32