param(
    [Parameter(Mandatory = $true)][string]$FilePath,

    [Parameter(Mandatory = $true)]
    [ValidateSet("source", "header")]
    [String]$Type
)

$FilePath = $FilePath.TrimEnd('\','/')
$normalized = $FilePath -replace '\\','/'

$filename = Split-Path $normalized -Leaf

if($normalized -match 'shaders/include/(.*)')
{
    $relPath = $Matches[1]
}
elseif($normalized -match 'shaders/(.*)')
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

if($Type -eq "header")
{
    $content += @"
#ifndef $guard
#define $guard

#endif
"@
}
else
{
    $content += @"
#version 450
#extension GL_GOOGLE_include_directive : require

"@
}

Set-Content -Path $FilePath -Value $content -Encoding ascii