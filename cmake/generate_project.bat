@echo off
set /p GAME_NAME="Enter project name: "
cd ..
cmake -DGAME_NAME="%GAME_NAME%" -P cmake\generate-project.cmake
cd "%GAME_NAME%"
echo Please update cmake_user.txt manually
pause