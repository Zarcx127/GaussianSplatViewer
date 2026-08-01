param(
    [Parameter(Mandatory = $true)][string]$InputPath,
    [Parameter(Mandatory = $true)][string]$OutputPath,
    [Parameter(Mandatory = $true)][string]$ShaderName
)

$initializer = (Get-Content -Path $InputPath -Raw).Trim()

$symbolName = (($ShaderName + ".spv") -replace '[^a-zA-Z0-9_]', '_')

$content = @"
// GENERATED SHADER BINARY //

#pragma once

#include <cstddef>
#include <cstdint>

inline constexpr uint32_t $symbolName[] = $initializer;
inline constexpr size_t ${symbolName}_len = sizeof($symbolName);
"@

Set-Content -Path $OutputPath -Value $content -Encoding ascii
