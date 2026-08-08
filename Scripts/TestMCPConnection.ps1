$ErrorActionPreference = "Stop"
Add-Type -AssemblyName System.Net.Http

$Uri = "http://127.0.0.1:8018/mcp"
$Client = [System.Net.Http.HttpClient]::new()

function Send-McpRequest {
    param(
        [Parameter(Mandatory = $true)][string]$Json,
        [string]$SessionId
    )

    $Request = [System.Net.Http.HttpRequestMessage]::new(
        [System.Net.Http.HttpMethod]::Post,
        $Uri)
    $Request.Headers.Accept.ParseAdd("application/json, text/event-stream")
    if ($SessionId) {
        [void]$Request.Headers.TryAddWithoutValidation("Mcp-Session-Id", $SessionId)
        [void]$Request.Headers.TryAddWithoutValidation("MCP-Protocol-Version", "2025-11-25")
    }
    $Request.Content = [System.Net.Http.StringContent]::new(
        $Json,
        [System.Text.Encoding]::UTF8,
        "application/json")

    $Response = $Client.SendAsync($Request).GetAwaiter().GetResult()
    $Body = $Response.Content.ReadAsStringAsync().GetAwaiter().GetResult()
    if (-not $Response.IsSuccessStatusCode) {
        throw "MCP request returned HTTP $([int]$Response.StatusCode): $Body"
    }
    return [pscustomobject]@{ Response = $Response; Body = $Body }
}

try {
    $Initialize = @{
        jsonrpc = "2.0"
        id = 1
        method = "initialize"
        params = @{
            protocolVersion = "2025-11-25"
            capabilities = @{}
            clientInfo = @{ name = "combat-interface-lab-smoke-test"; version = "1.0" }
        }
    } | ConvertTo-Json -Depth 6 -Compress

    $InitResult = Send-McpRequest -Json $Initialize
    $InitBody = $InitResult.Body | ConvertFrom-Json
    if ($InitBody.jsonrpc -ne "2.0" -or $null -eq $InitBody.result) {
        throw "MCP initialize returned an invalid JSON-RPC response."
    }
    $SessionId = @($InitResult.Response.Headers.GetValues("Mcp-Session-Id"))[0]

    $Initialized = '{"jsonrpc":"2.0","method":"notifications/initialized","params":{}}'
    [void](Send-McpRequest -Json $Initialized -SessionId $SessionId)

    $ListTools = '{"jsonrpc":"2.0","id":2,"method":"tools/list","params":{}}'
    $ToolsResult = Send-McpRequest -Json $ListTools -SessionId $SessionId
    $ToolNames = @(($ToolsResult.Body | ConvertFrom-Json).result.tools | ForEach-Object { $_.name })
    foreach ($RequiredTool in @("list_toolsets", "describe_toolset", "call_tool")) {
        if ($ToolNames -notcontains $RequiredTool) {
            throw "Required MCP meta-tool '$RequiredTool' was not advertised."
        }
    }

    Write-Host "Unreal MCP handshake and tools/list succeeded at $Uri"
    Write-Host "Session: $SessionId"
    Write-Host "Advertised tools: $($ToolNames -join ', ')"
}
finally {
    $Client.Dispose()
}
