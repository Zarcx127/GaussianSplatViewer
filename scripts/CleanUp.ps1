param(
    [string]$path,
    [string]$root
)

try 
{
    $resolvedPath = Resolve-Path -LiteralPath $path -ErrorAction Stop
    $path = $resolvedPath.Path
} 
catch { exit }

try 
{
    $resolvedRoot = Resolve-Path -LiteralPath $root -ErrorAction Stop
    $root = $resolvedRoot.Path
} 
catch { exit }

$path = $path.TrimEnd('\','/')
$root = $root.TrimEnd('\','/')

while(
    $path -and (Test-Path $path) -and 
    -not (Get-ChildItem -Force -ErrorAction SilentlyContinue $path)
) {
    if($path.ToLower() -eq $root.ToLower()) { break }

    Remove-Item -Force -Recurse $path
    $path = Split-Path $path -Parent
}
