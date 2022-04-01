rescan
boomdos data\level.boo %1 %2 %3 %4
@echo off
echo CPU type:
config -get "cpu cputype"
echo CPU core:
config -get "cpu core"
echo FPU enabled:
config -get "cpu fpu"