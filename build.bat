@echo off
Setlocal EnableDelayedExpansion

set check_failed=0

echo Starting HeaderTool Dependency Check...
echo ---------------------------------------

if exist clang\include\clang (
  echo -- Found clang includes folder.
) else (
  echo -- Failed to find clang includes in clang\include\
  set check_failed=1
)

if exist clang\include\clang-c (
  echo -- Found clang-c includes folder.
) else (
  echo -- Failed to find clang-c includes in clang\include\
  set check_failed=1
)

if exist clang\lib\libclang.lib (
	echo -- Found libclang.lib static library.
) else (
	echo -- Failed to find libclang.lib in clang\lib
	set check_failed=1
)

echo:

if %check_failed%==1 (
	echo HeaderTool Dependency Check Failed.
	goto failed
) else (
	echo HeaderTool Dependency Check Passed.
)

echo:
echo Starting Build...
echo ---------------------------------------
echo:

cmake -B ./Build
cd Build
cmake --build . --config Release

:failed
pause
