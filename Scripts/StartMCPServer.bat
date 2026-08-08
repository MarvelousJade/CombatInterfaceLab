@echo off
setlocal

if not defined UE58_ROOT set "UE58_ROOT=G:\Program Files\Epic Games\UE_5.8"
set "PROJECT=%~dp0..\CombatInterfaceLab.uproject"

if not exist "%UE58_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe" (
    echo UE 5.8 was not found. Set UE58_ROOT to the engine installation directory.
    exit /b 1
)

start "Combat Interface Lab MCP" "%UE58_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe" "%PROJECT%" -ModelContextProtocolStartServer -ModelContextProtocolPort=8018

echo Unreal MCP will be available after editor startup at http://127.0.0.1:8018/mcp
endlocal
