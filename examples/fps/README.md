# fps

This is a `YourGameLib` project, initialized from commit:

    2b1f7672ee18c2ce6aafe8e1c588c5a1a40cd091

Visit <https://github.com/duddel/yourgamelib> for more information.

# Compile
- install llvm-msvc
- click examples\fps\build\build_desktop_release_clang.cmd
- use vs2022 to open examples\fps\build\_build_release_clang\fps.sln
- Fill Additional Dependencies



/MD
```
ucrt.lib
msvcrt.lib
vcruntime.lib
```

/MT
```
libucrt.lib
libcpmt.lib
libvcruntime.lib
```


```
kernel32.lib
user32.lib
gdi32.lib
winspool.lib
comdlg32.lib
advapi32.lib
shell32.lib
ole32.lib
oleaut32.lib
uuid.lib
odbc32.lib
odbccp32.lib
libucrt.lib
libcpmt.lib
libvcruntime.lib
```
- Set Link Library Denpendencies

