$ErrorActionPreference = "Stop"
$secrets = "D:\design and development of a low geothermal\firmware\GeothermalMonitor\Secrets.h"
$username = (Select-String -Path $secrets -Pattern '#define\s+IO_USERNAME\s+"([^"]+)"').Matches[0].Groups[1].Value
$key = (Select-String -Path $secrets -Pattern '#define\s+IO_KEY\s+"([^"]+)"').Matches[0].Groups[1].Value
$headers = @{ "X-AIO-Key" = $key; "Accept" = "application/json" }
$postHeaders = @{ "X-AIO-Key" = $key; "Content-Type" = "application/json; charset=utf-8"; "Accept" = "application/json" }
$api = "https://io.adafruit.com/api/v2/$username"
$dashKey = "geothermal-fyp"

Write-Host "Verifying delta-t feed..."
try {
  Invoke-RestMethod -Uri "$api/feeds/delta-t" -Headers $headers -Method GET | Out-Null
  Write-Host "feed exists: delta-t"
} catch {
  $b = @{ feed = @{ name = "Thermal Differential (Delta T)"; key = "delta-t"; description = "Tinlet - Toutlet" } } | ConvertTo-Json -Compress
  Invoke-RestMethod -Uri "$api/feeds" -Headers $postHeaders -Method POST -Body $b | Out-Null
  Write-Host "feed created: delta-t"
}

# Clear any remaining blocks
Write-Host "Clearing old blocks..."
$oldBlocks = Invoke-RestMethod -Uri "$api/dashboards/$dashKey/blocks" -Headers $headers -Method GET
foreach ($ob in $oldBlocks) {
  try {
    Invoke-RestMethod -Uri "$api/dashboards/$dashKey/blocks/$($ob.id)" -Headers $headers -Method DELETE | Out-Null
    Start-Sleep -Milliseconds 150
  } catch {}
}

Function Create-DashBlock($bName, $vType, $col, $row, $sx, $sy, $feeds, $props) {
  $bodyObj = @{
    block = @{
      name = $bName
      visual_type = $vType
      column = $col
      row = $row
      size_x = $sx
      size_y = $sy
    }
  }
  if ($feeds -ne $null) {
    $bodyObj.block.block_feeds = @($feeds | ForEach-Object { @{ feed_id = $_ } })
  }
  if ($props -ne $null) {
    $bodyObj.block.properties = $props
  }

  $json = $bodyObj | ConvertTo-Json -Depth 6 -Compress
  try {
    Invoke-RestMethod -Uri "$api/dashboards/$dashKey/blocks" -Headers $postHeaders -Method POST -Body $json | Out-Null
    Write-Host "+ Block: $bName"
    Start-Sleep -Milliseconds 250
  } catch {
    Write-Host "! Failed $bName : $($_.Exception.Message)"
  }
}

Write-Host "Building Professional Layout..."

# Row 0: Banner
$bannerText = "# Shallow Geothermal Heat Exchanger (HGHE) Telemetry`n### Advanced Subsurface Slinky-Coil Monitoring Platform`n**Designed & Engineered by Engr. Hanif Khan**"
Create-DashBlock "Header Banner" "text" 0 0 12 2 $null @{ value = $bannerText }

# Row 2: Section 1 Header
Create-DashBlock "Section 1: Fluid Circuit Header" "text" 0 2 12 1 $null @{ value = "### 1. Primary Geothermal Fluid Circuit (Inlet / Outlet / Delta T)" }

# Row 3: Water Loop Gauges
Create-DashBlock "Inlet Water (T_in)" "gauge" 0 3 4 4 @("inlet-temp") @{ minValue = "0"; maxValue = "50"; label = "C"; ringWidth = "22" }
Create-DashBlock "Outlet Water (T_out)" "gauge" 4 3 4 4 @("outlet-temp") @{ minValue = "0"; maxValue = "50"; label = "C"; ringWidth = "22" }
Create-DashBlock "Thermal Drop (Delta T)" "gauge" 8 3 4 4 @("delta-t") @{ minValue = "-10"; maxValue = "15"; label = "C"; ringWidth = "22" }

# Row 7: Section 2 Header
Create-DashBlock "Section 2: Subsurface Soil Header" "text" 0 7 12 1 $null @{ value = "### 2. Subsurface Ground Profile & Pipe-Soil Interface" }

# Row 8: Ground and Pipe
Create-DashBlock "Deep Ground Soil (2.5m Depth)" "gauge" 0 8 6 4 @("ground-temp") @{ minValue = "0"; maxValue = "50"; label = "C"; ringWidth = "22" }
Create-DashBlock "HDPE Pipe Wall Surface" "gauge" 6 8 6 4 @("pipe-wall-temp") @{ minValue = "0"; maxValue = "50"; label = "C"; ringWidth = "22" }

# Row 12: Section 3 Header
Create-DashBlock "Section 3: Environment Header" "text" 0 12 12 1 $null @{ value = "### 3. Ambient Weather & Conditioned Indoor Environment" }

# Row 13: Environment Gauges
Create-DashBlock "Conditioned Room Air" "gauge" 0 13 6 4 @("room-temp") @{ minValue = "0"; maxValue = "50"; label = "C"; ringWidth = "22" }
Create-DashBlock "Outdoor Ambient Air" "gauge" 6 13 6 4 @("outdoor-temp") @{ minValue = "0"; maxValue = "50"; label = "C"; ringWidth = "22" }

# Row 17: Section 4 Header
Create-DashBlock "Section 4: Analytics Header" "text" 0 17 12 1 $null @{ value = "### 4. 24-Hour Comparative Thermal Performance Charts" }

# Row 18: Analytical Charts
Create-DashBlock "Geothermal Loop & Soil Thermal Response (24h)" "line_chart" 0 18 6 6 @("inlet-temp", "outlet-temp", "pipe-wall-temp", "ground-temp") @{ xAxisLabel = "Time"; yAxisLabel = "C"; yAxisMin = "0"; yAxisMax = "50"; historyHours = "24" }
Create-DashBlock "Outdoor Ambient vs Indoor Room Response (24h)" "line_chart" 6 18 6 6 @("outdoor-temp", "room-temp") @{ xAxisLabel = "Time"; yAxisLabel = "C"; yAxisMin = "0"; yAxisMax = "50"; historyHours = "24" }

# Row 24: Footer
$footerText = "**Heat Extraction Rate Formula:** Q = m * Cp * (Tin - Tout)  |  **System Engineering & IoT Development:** Engr. Hanif Khan (+92 311 8221686 / Hanifullahshams@gmail.com)"
Create-DashBlock "Project Footer" "text" 0 24 12 2 $null @{ value = $footerText }

Write-Host "`nExecutive Dashboard successfully built!"
Write-Host "Dashboard: https://io.adafruit.com/$username/dashboards/$dashKey"
