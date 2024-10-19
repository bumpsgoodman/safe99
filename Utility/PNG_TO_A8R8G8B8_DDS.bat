for %%f in (*.png) do (
  .\texconv.exe -f B8G8R8A8_UNORM -srgb -ft dds "%%~f"
)
