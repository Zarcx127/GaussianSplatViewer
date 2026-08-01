param(
    [Parameter(Mandatory = $true)][string]$FilePath,
    [Parameter(Mandatory = $true)][string]$Type
)

$FilePath = $FilePath.TrimEnd('\','/')
$normalized = $FilePath -replace '\\','/'

$filename = Split-Path $normalized -Leaf

if($normalized -match 'src/(.*)') 
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

$guardBuilder = New-Object System.Text.StringBuilder
$prevWasUnderscore = $false
$index = 0

foreach($ch in $filename.ToCharArray()) 
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

if($Type -eq "hpp") 
{
    $content = @"
#pragma once

#ifndef $guard
#define $guard

#endif
"@
}
else
{
    $includePath = "$relPath.hpp"
    $content = @"
#include `"$includePath`"

#ifdef $guard

#endif
"@
}

$target = "$FilePath.$Type"
Set-Content -Path $target -Value $content
