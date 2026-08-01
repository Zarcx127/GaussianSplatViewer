param(
    [Parameter(Mandatory = $true)][string]$FilePath,
    
    [Parameter(Mandatory = $true)]
    [ValidateSet("hpp", "cpp")]
    [String]$Type
)

$FilePath = $FilePath.TrimEnd('\','/')
$normalized = $FilePath -replace '\\','/'

$filename = Split-Path $normalized -Leaf

if($normalized -match 'source/(.*)') 
{
    $relPath = $Matches[1]
}
elseif($normalized -match 'include/(.*)') 
{
    $relPath = $Matches[1]
}
else 
{
    $relPath = $filename
}

$guardSource = $relPath -replace '\\', '/'
$guardSource = $guardSource -replace '/', '_'
$guardSource = $relPath -replace '[^a-zA-Z0-9_]', '_'

$guardBuilder = New-Object System.Text.StringBuilder
$prevWasUnderscore = $false
$index = 0

foreach($ch in $guardSource.ToCharArray()) 
{
    $isUpper = ($ch -cmatch '^[A-Z]$')
    $isUnderscore = ($ch -ceq '_')

    if($isUnderscore) 
    {
        [void]$guardBuilder.Append('_')
        $prevWasUnderscore = $true
    }
    else 
    {
        if($isUpper -and $index -gt 0 -and -not $prevWasUnderscore) 
        {
            [void]$guardBuilder.Append('_')
        }

        [void]$guardBuilder.Append($ch)
        $prevWasUnderscore = $false
    }

    $index++
}

$guard = $guardBuilder.ToString().ToUpper()
$guard = "${guard}_H"

$dir = Split-Path $FilePath -Parent
if($dir -and -not (Test-Path $dir)) 
{
    New-Item -ItemType Directory -Path $dir | Out-Null
}


$content = @"
/**
 * Copyright (C) 2026  Zarcx127@github.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 **/
`n
"@

if($Type -eq "hpp") 
{
    $content += @"
#pragma once

#ifndef $guard
#define $guard

#endif
"@
}
else
{
    $headerPath = ($normalized -replace '(^|/)source/', '${1}include/') + '.hpp'
    if(Test-Path -LiteralPath $headerPath -PathType Leaf)
    {
        $includePath = "$relPath.hpp"
        $content += @"
#include `"$includePath`"

"@
    }
    else
    {
        $content += @"
// no matching header
"@
        Write-Warning "Corresponding header does not exist"
    }
}

$target = "$FilePath.$Type"
Set-Content -Path $target -Value $content -Encoding ascii
