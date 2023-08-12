@echo off
setlocal

:: Get the number of logical processors
for /f "tokens=*" %%i in ('wmic cpu get NumberOfLogicalProcessors /value ^| find "NumberOfLogicalProcessors"') do set %%i

:: Subtract 1 from the number of logical processors, with a minimum of 1
set /a processors=%NumberOfLogicalProcessors%-1
if %processors% lss 1 set processors=1

echo Number of Logical Processors: %NumberOfLogicalProcessors%
echo Using %processors% processors for compilation

:: Compile flags
set COMPILE_FLAGS=/c /fp:fast /favor:AMD64 /Ox /Ob2 /Qpar /nologo /openmp:experimental /MP%processors% /MT /TC /D"COMPILER=\"MSVC\"" /I. /I"C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt" /I"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.36.32532\include" /I"C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\um" /I"C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\shared" run.c win.c base64_utils.c save_to_file.c
echo Compile Flags: %COMPILE_FLAGS%
cl.exe %COMPILE_FLAGS%

:: Link flags
set LINK_FLAGS=/out:runmsvc.exe /nologo /LIBPATH:"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.36.32532\lib\x64" /LIBPATH:"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.19041.0\um\x64" /LIBPATH:"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.19041.0\ucrt\x64" run.obj win.obj base64_utils.obj save_to_file.obj
echo Link Flags: %LINK_FLAGS%
"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.36.32532\bin\Hostx64\x64\link.exe" %LINK_FLAGS%

endlocal
