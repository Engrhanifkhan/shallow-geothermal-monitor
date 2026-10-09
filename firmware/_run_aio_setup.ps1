$ErrorActionPreference = "Stop"
$secrets = "D:\design and development of a low geothermal\firmware\GeothermalMonitor\Secrets.h"
$username = (Select-String -Path $secrets -Pattern '#define\s+IO_USERNAME\s+"([^"]+)"').Matches[0].Groups[1].Value
$key = (Select-String -Path $secrets -Pattern '#define\s+IO_KEY\s+"([^"]+)"').Matches[0].Groups[1].Value
$headers = @{ "X-AIO-Key" = $key; "Accept" = "application/json" }
$postHeaders = @{ "X-AIO-Key" = $key; "Content-Type" = "application/json; charset=utf-8"; "Accept" = "application/json" }
$api = "https://io.adafruit.com/api/v2/$username"

try {
  $me = Invoke-RestMethod -Uri "https://io.adafruit.com/api/v2/user" -Headers $headers -Method GET
  Write-Host ("connected as " + $me.user.username)
} catch {
  Write-Host ("AUTH FAIL: " + $_.Exception.Message)
  if ($_.ErrorDetails) { Write-Host $_.ErrorDetails.Message }
  exit 1
}

$feeds = @(
  @{ name = "Room temperature"; key = "room-temp" },
  @{ name = "Outdoor temperature"; key = "outdoor-temp" },
  @{ name = "Inlet water temperature"; key = "inlet-temp" },
  @{ name = "Outlet water temperature"; key = "outlet-temp" },
  @{ name = "Pipe wall temperature"; key = "pipe-wall-temp" },
  @{ name = "Ground temperature"; key = "ground-temp" }
)

foreach ($f in $feeds) {
  try {
    Invoke-RestMethod -Uri ($api + "/feeds/" + $f.key) -Headers $headers -Method GET | Out-Null
    Write-Host ("feed exists: " + $f.key)
  } catch {
    $body = @{ feed = @{ name = $f.name; key = $f.key; description = $f.name } } | ConvertTo-Json -Compress
    try {
      Invoke-RestMethod -Uri ($api + "/feeds") -Headers $postHeaders -Method POST -Body $body | Out-Null
      Write-Host ("feed created: " + $f.key)
    } catch {
      Write-Host ("feed FAIL " + $f.key + ": " + $_.Exception.Message)
      if ($_.ErrorDetails) { Write-Host $_.ErrorDetails.Message }
      exit 1
    }
  }
}

$dashKey = "geothermal-fyp"
try {
  Invoke-RestMethod -Uri ($api + "/dashboards/" + $dashKey) -Headers $headers -Method GET | Out-Null
  Write-Host ("dashboard exists: " + $dashKey)
} catch {
  $body = @{ dashboard = @{ name = "Geothermal FYP"; key = $dashKey; description = "ESP32 DS18B20 shallow geothermal monitor" } } | ConvertTo-Json -Compress
  $created = Invoke-RestMethod -Uri ($api + "/dashboards") -Headers $postHeaders -Method POST -Body $body
  $dashKey = $created.key
  Write-Host ("dashboard created: " + $dashKey)
}

$existing = @()
try {
  $blocks = Invoke-RestMethod -Uri ($api + "/dashboards/" + $dashKey + "/blocks") -Headers $headers -Method GET
  if ($blocks) { $existing = @($blocks | ForEach-Object { $_.name }) }
} catch {
  $existing = @()
}

$i = 0
foreach ($f in $feeds) {
  $gaugeName = $f.name + " gauge"
  if ($existing -contains $gaugeName) {
    Write-Host ("block exists: " + $gaugeName)
  } else {
    $payload = @{
      block = @{
        name = $gaugeName
        visual_type = "gauge"
        column = ($i * 2)
        row = 0
        size_x = 2
        size_y = 4
        block_feeds = @(@{ feed_id = $f.key })
        properties = @{ minValue = "0"; maxValue = "50"; label = "C"; ringWidth = "25" }
      }
    } | ConvertTo-Json -Depth 6 -Compress
    try {
      Invoke-RestMethod -Uri ($api + "/dashboards/" + $dashKey + "/blocks") -Headers $postHeaders -Method POST -Body $payload | Out-Null
      Write-Host ("block created: " + $gaugeName)
    } catch {
      Write-Host ("block FAIL " + $gaugeName + " " + $_.Exception.Message)
      if ($_.ErrorDetails) { Write-Host $_.ErrorDetails.Message }
    }
  }
  $i++
}

$chartName = "All temperatures 24h"
if ($existing -contains $chartName) {
  Write-Host ("block exists: " + $chartName)
} else {
  $feedIds = @($feeds | ForEach-Object { @{ feed_id = $_.key } })
  $payload = @{
    block = @{
      name = $chartName
      visual_type = "line_chart"
      column = 0
      row = 4
      size_x = 12
      size_y = 6
      block_feeds = $feedIds
      properties = @{ xAxisLabel = "Time"; yAxisLabel = "C"; yAxisMin = "0"; yAxisMax = "50"; historyHours = "24" }
    }
  } | ConvertTo-Json -Depth 6 -Compress
  try {
    Invoke-RestMethod -Uri ($api + "/dashboards/" + $dashKey + "/blocks") -Headers $postHeaders -Method POST -Body $payload | Out-Null
    Write-Host ("block created: " + $chartName)
  } catch {
    Write-Host ("block FAIL chart " + $_.Exception.Message)
    if ($_.ErrorDetails) { Write-Host $_.ErrorDetails.Message }
  }
}

Write-Host ("Dashboard: https://io.adafruit.com/" + $username + "/dashboards/" + $dashKey)
