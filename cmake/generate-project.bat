@echo off
set batdir=%~dp0
echo NOTE: Project will be created in %CD%\GAME_NAME
echo.
set /p GAME_NAME="Enter project name: "
cmake -DGAME_NAME="%GAME_NAME%" -P %batdir%\generate-project.cmake
cd "%GAME_NAME%"
echo Please update cmake_user.txt manually
pause