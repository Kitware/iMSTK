@echo off
rem We avoid using the uncrustify command for a list of files as it is buggy

rem This script accepts the directory to uncrustify
set uncrustifyDir=%1
echo Uncrustify Directory: %uncrustifyDir%

rem Save the directory the user had before calling
set startDir=%CD%
set thisScriptDir=%~dp0
echo Start Directory: %startDir%
echo This Script Directory: %thisScriptDir%

rem Get the source directory (should be two levels up from this file)
cd..
cd..
set sourcedir=%CD%

rem Begin uncrustifying
echo Uncrusting: %sourcedir%\Source
cd %sourcedir%\Source
call :treeProcess
echo Uncrusting: %sourcedir%\Examples
cd %sourcedir%\Examples
call :treeProcess
cd %startDir%
goto :eof


:treeProcess

rem For every file in the directory
for %%f in (*.cpp) do (
    rem echo Using: %CD%\%%f
    %uncrustifyDir%/uncrustify -c %thisScriptDir%iMSTKUncrustify.cfg --no-backup %CD%\%%f
)
for %%f in (*.h) do (
    rem echo Using: %CD%\%%f
    %uncrustifyDir%/uncrustify -c %thisScriptDir%iMSTKUncrustify.cfg --no-backup %CD%\%%f
)

rem For every directory, recurse
for /D %%d in (*) do (
    cd %%d
    call :treeProcess
    cd ..
)
exit /b