@echo off
setlocal enableextensions
set VALS_CONFIG=debug release
set VALS_CPU=386 586
set VALS_SYS=dos

for %%i in (%VALS_CONFIG%) do (
    for %%j in (%VALS_CPU%) do (
        for %%k in (%VALS_SYS%) do (
            echo on
            wmake -h CONFIG=%%i CPU=%%j TARGET_SYS=%%k
            @if errorlevel 1 @exit /b %errorlevel%
            @echo off
        )
    )
)