# Unreal MCP workflow (UE 5.8)

The project enables Epic's **experimental Unreal MCP server** only for Editor targets. It is development tooling and is not included in packaged games.

## Enabled toolsets

- `EditorToolset` — actors, assets, Blueprints, properties, and editor operations
- `UMGToolSet` — UMG creation and reflection-based widget editing
- `SlateInspectorToolset` — Slate inspection and UI automation
- `AutomationTestToolset` — test discovery and execution

Tool search remains enabled, so clients initially receive the compact `list_toolsets`, `describe_toolset`, and `call_tool` interface rather than every editor tool.

## Start and connect

1. Run `Scripts/StartMCPServer.bat`, or open the project normally in UE 5.8. The per-project setting auto-starts the server.
2. Connect an MCP HTTP client to `http://127.0.0.1:8018/mcp`.
3. Optionally run `Scripts/TestMCPConnection.ps1` to verify the MCP initialize handshake.

The checked-in `.mcp.json` is compatible with clients that use Claude-style project MCP configuration.

Useful editor console commands:

```text
ModelContextProtocol.StartServer 8018
ModelContextProtocol.StopServer
ModelContextProtocol.RefreshTools
ModelContextProtocol.GenerateClientConfig ClaudeCode
```

The port can be overridden when starting the editor:

```text
-ModelContextProtocolStartServer -ModelContextProtocolPort=8010
```

## Safety

The server can execute editor tools that modify assets. Run it only on a trusted workstation, keep source control enabled, inspect changes before committing, and stop the server when it is not needed. Do not enable the plugin for Game or Server targets.
