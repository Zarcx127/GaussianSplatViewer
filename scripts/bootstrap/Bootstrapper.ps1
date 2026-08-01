# AI GENERATED #

param(
    [Parameter(Mandatory = $true)]
    [string]$ProjectRoot
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

[Net.ServicePointManager]::SecurityProtocol =
    [Net.ServicePointManager]::SecurityProtocol -bor
    [Net.SecurityProtocolType]::Tls12

$root = [IO.Path]::GetFullPath($ProjectRoot).TrimEnd("\", "/")

if (-not (Test-Path -LiteralPath $root -PathType Container)) {
    throw "Project root does not exist: $root"
}

$cache = [IO.Path]::GetFullPath((Join-Path $root ".setup-cache"))
$toolchain = [IO.Path]::GetFullPath((Join-Path $root ".toolchain"))

$ucrt64 = [IO.Path]::GetFullPath((Join-Path $toolchain "ucrt64"))
$ucrtBin = [IO.Path]::GetFullPath((Join-Path $ucrt64 "bin"))
$ucrtInclude = [IO.Path]::GetFullPath((Join-Path $ucrt64 "include"))
$ucrtLib = [IO.Path]::GetFullPath((Join-Path $ucrt64 "lib"))
$ucrtTargetLib = [IO.Path]::GetFullPath(
    (Join-Path $ucrt64 "x86_64-w64-mingw32\lib")
)

$vulkan = [IO.Path]::GetFullPath((Join-Path $toolchain "vulkan"))
$vulkanBin = [IO.Path]::GetFullPath((Join-Path $vulkan "bin"))
$vulkanInclude = [IO.Path]::GetFullPath((Join-Path $vulkan "include"))
$vulkanLib = [IO.Path]::GetFullPath((Join-Path $vulkan "lib"))

New-Item -ItemType Directory -Force -Path $cache, $toolchain | Out-Null

function Download-File {
    param(
        [Parameter(Mandatory)][string]$url,
        [Parameter(Mandatory)][string]$output,
        [int]$maxAttempts = 3
    )

    if (Test-Path -LiteralPath $output -PathType Leaf) {
        Write-Host "Using cached $(Split-Path $output -Leaf)"
        return
    }

    $parent = Split-Path -Parent $output
    $partial = "$output.partial"

    if ($parent) {
        New-Item -ItemType Directory -Force -Path $parent | Out-Null
    }

    for ($attempt = 1; $attempt -le $maxAttempts; $attempt++) {
        Remove-Item -LiteralPath $partial -Force -ErrorAction SilentlyContinue
        Write-Host "Downloading $url"
        Write-Host "Attempt $attempt of $maxAttempts"

        try {
            Invoke-WebRequest `
                -UseBasicParsing `
                -Headers @{ "User-Agent" = "GaussianSplatViewer-Setup" } `
                -Uri $url `
                -OutFile $partial `
                -ErrorAction Stop

            if (-not (Test-Path -LiteralPath $partial -PathType Leaf)) {
                throw "Download completed without creating a file."
            }

            if ((Get-Item -LiteralPath $partial).Length -eq 0) {
                throw "Downloaded file is empty."
            }

            Move-Item -LiteralPath $partial -Destination $output -Force
            return
        }
        catch {
            Remove-Item -LiteralPath $partial -Force -ErrorAction SilentlyContinue

            if ($attempt -eq $maxAttempts) {
                throw "Download failed after $maxAttempts attempts.`nURL: $url`n$($_.Exception.Message)"
            }

            $delay = $attempt * 5
            Write-Warning "Download failed: $($_.Exception.Message)"
            Write-Host "Retrying in $delay seconds..."
            Start-Sleep -Seconds $delay
        }
    }
}

function Assert-Sha256 {
    param([string]$file, [string]$expected)

    $actual = (Get-FileHash -LiteralPath $file -Algorithm SHA256).Hash

    if ($actual -ne $expected.ToUpperInvariant()) {
        throw "SHA-256 verification failed.`nFile: $file`nExpected: $expected`nActual: $actual"
    }
}

function Expand-CleanZip {
    param([string]$archive, [string]$destination)

    Remove-Item -LiteralPath $destination -Recurse -Force -ErrorAction SilentlyContinue
    New-Item -ItemType Directory -Force -Path $destination | Out-Null
    Expand-Archive -LiteralPath $archive -DestinationPath $destination -Force
}

function Expand-TarZst {
    param([string]$archive, [string]$destination)

    $tar = Join-Path $env:SystemRoot "System32\tar.exe"

    if (-not (Test-Path -LiteralPath $tar -PathType Leaf)) {
        throw "Windows tar.exe was not found: $tar"
    }

    Remove-Item -LiteralPath $destination -Recurse -Force -ErrorAction SilentlyContinue
    New-Item -ItemType Directory -Force -Path $destination | Out-Null

    & $tar -xf $archive -C $destination

    if ($LASTEXITCODE -ne 0) {
        throw "Could not extract package archive: $archive"
    }
}

function Get-OnlyDirectory {
    param([string]$path)

    $directories = @(Get-ChildItem -LiteralPath $path -Directory)

    if ($directories.Count -ne 1) {
        throw "Expected exactly one top-level directory in: $path"
    }

    return $directories[0].FullName
}

function Get-RelativePath {
    param([string]$sourceRoot, [string]$sourcePath)

    $sourceRoot = [IO.Path]::GetFullPath($sourceRoot).TrimEnd("\", "/") + "\"
    $sourcePath = [IO.Path]::GetFullPath($sourcePath)

    if (-not $sourcePath.StartsWith($sourceRoot, [StringComparison]::OrdinalIgnoreCase)) {
        throw "Path is not inside source root.`nRoot: $sourceRoot`nPath: $sourcePath"
    }

    return $sourcePath.Substring($sourceRoot.Length)
}

function Copy-Tree {
    param([string]$source, [string]$destination)

    if (-not (Test-Path -LiteralPath $source -PathType Container)) {
        throw "Required directory not found: $source"
    }

    Remove-Item -LiteralPath $destination -Recurse -Force -ErrorAction SilentlyContinue
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $destination) | Out-Null
    Copy-Item -LiteralPath $source -Destination $destination -Recurse -Force
}

function Merge-Tree {
    param([string]$source, [string]$destination)

    if (-not (Test-Path -LiteralPath $source -PathType Container)) {
        throw "Required directory not found: $source"
    }

    New-Item -ItemType Directory -Force -Path $destination | Out-Null

    Get-ChildItem -LiteralPath $source -Recurse -Force | ForEach-Object {
        $target = Join-Path $destination (Get-RelativePath $source $_.FullName)

        if ($_.PSIsContainer) {
            New-Item -ItemType Directory -Force -Path $target | Out-Null
        }
        else {
            New-Item -ItemType Directory -Force -Path (Split-Path -Parent $target) | Out-Null
            Copy-Item -LiteralPath $_.FullName -Destination $target -Force
        }
    }
}

function Copy-RelativeFile {
    param(
        [string]$sourceRoot,
        [string]$sourceFile,
        [string]$destinationRoot
    )

    $destination = Join-Path $destinationRoot (Get-RelativePath $sourceRoot $sourceFile)
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $destination) | Out-Null
    Copy-Item -LiteralPath $sourceFile -Destination $destination -Force
}

function Test-AllFiles {
    param([string[]]$files)

    foreach ($file in $files) {
        if (-not (Test-Path -LiteralPath $file -PathType Leaf)) {
            return $false
        }
    }

    return $true
}

function Copy-PeDependencies {
    param(
        [string]$objdump,
        [string[]]$files,
        [string[]]$searchDirectories,
        [string]$destination
    )

    $queue = [Collections.Generic.Queue[string]]::new()
    $seen = [Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)

    foreach ($file in $files) {
        if (Test-Path -LiteralPath $file -PathType Leaf) {
            $queue.Enqueue((Resolve-Path -LiteralPath $file).Path)
        }
    }

    $oldPath = $env:PATH
    $env:PATH = (($searchDirectories + @($oldPath)) -join ";")

    try {
        while ($queue.Count -gt 0) {
            $file = $queue.Dequeue()

            if (-not $seen.Add($file)) {
                continue
            }

            $output = & $objdump -p $file 2>$null

            if ($LASTEXITCODE -ne 0) {
                throw "Could not inspect binary dependencies for: $file"
            }

            foreach ($line in $output) {
                if ($line -notmatch "^\s*DLL Name:\s*(.+?)\s*$") {
                    continue
                }

                $dllName = $Matches[1]

                foreach ($directory in $searchDirectories) {
                    $candidate = Join-Path $directory $dllName

                    if (-not (Test-Path -LiteralPath $candidate -PathType Leaf)) {
                        continue
                    }

                    $target = Join-Path $destination $dllName

                    if (-not (Test-Path -LiteralPath $target -PathType Leaf)) {
                        Copy-Item -LiteralPath $candidate -Destination $target -Force
                    }

                    $queue.Enqueue($candidate)
                    break
                }
            }
        }
    }
    finally {
        $env:PATH = $oldPath
    }
}

function Install-UcrtPackage {
    param(
        [hashtable]$package,
        [string]$destinationPrefix
    )

    $archive = Join-Path $cache $package.File
    $stage = Join-Path $cache "packages\$($package.Name)"
    $url = "https://mirror.msys2.org/mingw/ucrt64/$($package.File)"

    Download-File $url $archive
    Assert-Sha256 $archive $package.Sha256

    Write-Host "Extracting $($package.File)..."
    Expand-TarZst $archive $stage

    $sourcePrefix = Join-Path $stage "ucrt64"

    if (-not (Test-Path -LiteralPath $sourcePrefix -PathType Container)) {
        throw "Package did not contain the expected ucrt64 prefix: $($package.File)"
    }

    Merge-Tree $sourcePrefix $destinationPrefix
    Remove-Item -LiteralPath $stage -Recurse -Force
}

# ---------------------------------------------------------------------------
# UCRT64 compiler, C++ runtimes, binutils and GNU Make
# ---------------------------------------------------------------------------

$compilerVersion = "gcc-16.1.0-mingw-w64ucrt-14.0.0-r3"
$compilerTag = "16.1.0posix-14.0.0-ucrt-r3"
$compilerAsset = "winlibs-x86_64-posix-seh-gcc-16.1.0-mingw-w64ucrt-14.0.0-r3.zip"
$compilerUrl = "https://github.com/brechtsanders/winlibs_mingw/releases/download/$compilerTag/$compilerAsset"
$compilerSha256 = "4273565109CD8AB8ECEF1B0DC2A56FD7F5C7EE0885840A1C011B9325160EC0C3"

$compilerArchive = Join-Path $cache $compilerAsset
$compilerExtract = Join-Path $cache "winlibs"
$compilerMarker = Join-Path $ucrt64 ".compiler-version"

$compilerFiles = @(
    (Join-Path $ucrtBin "g++.exe"),
    (Join-Path $ucrtBin "gcc.exe"),
    (Join-Path $ucrtBin "cpp.exe"),
    (Join-Path $ucrtBin "as.exe"),
    (Join-Path $ucrtBin "ld.exe"),
    (Join-Path $ucrtBin "ar.exe"),
    (Join-Path $ucrtBin "ranlib.exe"),
    (Join-Path $ucrtBin "dlltool.exe"),
    (Join-Path $ucrtBin "windres.exe"),
    (Join-Path $ucrtBin "mingw32-make.exe"),
    (Join-Path $ucrtBin "make.exe"),
    (Join-Path $ucrtBin "objdump.exe"),
    (Join-Path $ucrtBin "libgcc_s_seh-1.dll"),
    (Join-Path $ucrtBin "libstdc++-6.dll"),
    (Join-Path $ucrtBin "libwinpthread-1.dll"),

    (Join-Path $ucrtTargetLib "libgdi32.a"),
    (Join-Path $ucrtTargetLib "libole32.a"),
    (Join-Path $ucrtTargetLib "libuuid.a"),
    (Join-Path $ucrtTargetLib "libdwmapi.a")
)

$compilerReady = (Test-AllFiles $compilerFiles) -and
    (Test-Path -LiteralPath $compilerMarker -PathType Leaf) -and
    ((Get-Content -LiteralPath $compilerMarker -Raw).Trim() -eq $compilerVersion)

if (-not $compilerReady) {
    Download-File $compilerUrl $compilerArchive
    Assert-Sha256 $compilerArchive $compilerSha256

    Write-Host "Extracting UCRT64 toolchain..."
    Expand-CleanZip $compilerArchive $compilerExtract

    $mingwRoot = Join-Path $compilerExtract "mingw64"
    $sourceBin = Join-Path $mingwRoot "bin"
    $sourceLibexec = Join-Path $mingwRoot "libexec"

    if (-not (Test-Path -LiteralPath $mingwRoot -PathType Container)) {
        throw "The WinLibs archive did not contain mingw64\"
    }

    Remove-Item -LiteralPath $ucrt64 -Recurse -Force -ErrorAction SilentlyContinue
    New-Item -ItemType Directory -Force -Path $ucrt64, $ucrtBin | Out-Null

    foreach ($relativePath in @(
        "include",
        "lib",
        "x86_64-w64-mingw32\include",
        "x86_64-w64-mingw32\lib"
    )) {
        Copy-Tree `
            (Join-Path $mingwRoot $relativePath) `
            (Join-Path $ucrt64 $relativePath)
    }

    $defaultManifest = Join-Path $ucrtTargetLib "default-manifest.o"

    if (Test-Path -LiteralPath $defaultManifest -PathType Leaf) {
        Remove-Item -LiteralPath $defaultManifest -Force
    }

    $requiredPrograms = @(
        "g++.exe",
        "gcc.exe",
        "cpp.exe",
        "as.exe",
        "ld.exe",
        "ar.exe",
        "ranlib.exe",
        "dlltool.exe",
        "windres.exe",
        "mingw32-make.exe",
        "objdump.exe"
    )

    foreach ($program in $requiredPrograms) {
        $source = Join-Path $sourceBin $program

        if (-not (Test-Path -LiteralPath $source -PathType Leaf)) {
            throw "Required toolchain program not found: $source"
        }

        Copy-Item -LiteralPath $source -Destination $ucrtBin -Force
    }

    foreach ($dll in @(
        "libgcc_s_seh-1.dll",
        "libstdc++-6.dll",
        "libwinpthread-1.dll"
    )) {
        $source = Join-Path $sourceBin $dll

        if (-not (Test-Path -LiteralPath $source -PathType Leaf)) {
            throw "Required compiler runtime DLL not found: $source"
        }

        Copy-Item -LiteralPath $source -Destination $ucrtBin -Force
    }

    Copy-Item `
        -LiteralPath (Join-Path $ucrtBin "mingw32-make.exe") `
        -Destination (Join-Path $ucrtBin "make.exe") `
        -Force

    foreach ($name in @(
        "cc1.exe",
        "cc1plus.exe",
        "collect2.exe"
    )) {
        $matches = @(
            Get-ChildItem `
                -LiteralPath $sourceLibexec `
                -Recurse `
                -File `
                -Filter $name
        )

        if ($matches.Count -eq 0) {
            throw "Required compiler component not found: $name"
        }

        foreach ($file in $matches) {
            Copy-RelativeFile $mingwRoot $file.FullName $ucrt64
        }
    }

    foreach ($pattern in @(
        "lto1.exe",
        "lto-wrapper.exe",
        "liblto_plugin*.dll"
    )) {
        Get-ChildItem `
            -LiteralPath $sourceLibexec `
            -Recurse `
            -File `
            -Filter $pattern |
            ForEach-Object {
                Copy-RelativeFile $mingwRoot $_.FullName $ucrt64
            }
    }

    $dependencyDirectories = @($sourceBin)
    $dependencyDirectories += @(
        Get-ChildItem `
            -LiteralPath $sourceLibexec `
            -Recurse `
            -Directory |
            ForEach-Object {
                $_.FullName
            }
    )

    $compilerPeFiles = @(
        Get-ChildItem `
            -LiteralPath $ucrt64 `
            -Recurse `
            -File |
            Where-Object {
                $_.Extension -in @(".exe", ".dll")
            } |
            ForEach-Object {
                $_.FullName
            }
    )

    Copy-PeDependencies `
        (Join-Path $sourceBin "objdump.exe") `
        $compilerPeFiles `
        $dependencyDirectories `
        $ucrtBin

    Set-Content -LiteralPath $compilerMarker -Value $compilerVersion -NoNewline
    Remove-Item -LiteralPath $compilerExtract -Recurse -Force
}
else {
    Write-Host "UCRT64 compiler toolchain already present."
}

# ---------------------------------------------------------------------------
# GLFW, preserving its complete UCRT64 package layout
# ---------------------------------------------------------------------------

$glfwVersion = "3.4-1"
$glfwMarker = Join-Path $ucrt64 ".glfw-version"

$glfwPackage = @{
    Name = "glfw"
    File = "mingw-w64-ucrt-x86_64-glfw-3.4-1-any.pkg.tar.zst"
    Sha256 = "DE9F2B0903D56446EBF340AD444188F105361DA218078C11FA9052EB283377CE"
}

$glfwFiles = @(
    (Join-Path $ucrtBin "glfw3.dll"),
    (Join-Path $ucrtInclude "GLFW\glfw3.h"),
    (Join-Path $ucrtInclude "GLFW\glfw3native.h"),
    (Join-Path $ucrtLib "libglfw3.a"),
    (Join-Path $ucrtLib "libglfw3.dll.a")
)

$glfwReady = (Test-AllFiles $glfwFiles) -and
    (Test-Path -LiteralPath $glfwMarker -PathType Leaf) -and
    ((Get-Content -LiteralPath $glfwMarker -Raw).Trim() -eq $glfwVersion)

if (-not $glfwReady) {
    Install-UcrtPackage $glfwPackage $ucrt64

    Copy-PeDependencies `
        (Join-Path $ucrtBin "objdump.exe") `
        @((Join-Path $ucrtBin "glfw3.dll")) `
        @($ucrtBin) `
        $ucrtBin

    Set-Content -LiteralPath $glfwMarker -Value $glfwVersion -NoNewline
}
else {
    Write-Host "GLFW already present."
}

# ---------------------------------------------------------------------------
# GLM
# ---------------------------------------------------------------------------

$glmVersion = "1.0.3"
$glmUrl = "https://github.com/g-truc/glm/archive/refs/tags/$glmVersion.zip"
$glmArchive = Join-Path $cache "glm-$glmVersion.zip"
$glmExtract = Join-Path $cache "glm"
$glmMarker = Join-Path $ucrt64 ".glm-version"
$glmHeader = Join-Path $ucrtInclude "glm\glm.hpp"

$glmReady = (Test-AllFiles @($glmHeader)) -and
    (Test-Path -LiteralPath $glmMarker -PathType Leaf) -and
    ((Get-Content -LiteralPath $glmMarker -Raw).Trim() -eq $glmVersion)

if (-not $glmReady) {
    Download-File $glmUrl $glmArchive

    Write-Host "Extracting GLM..."
    Expand-CleanZip $glmArchive $glmExtract

    $glmRoot = Get-OnlyDirectory $glmExtract
    Copy-Tree `
        (Join-Path $glmRoot "glm") `
        (Join-Path $ucrtInclude "glm")

    $glmLicense = Join-Path $glmRoot "copying.txt"

    if (Test-Path -LiteralPath $glmLicense -PathType Leaf) {
        $glmLicenseDestination = Join-Path $ucrt64 "share\licenses\glm"
        New-Item -ItemType Directory -Force -Path $glmLicenseDestination | Out-Null
        Copy-Item -LiteralPath $glmLicense -Destination $glmLicenseDestination -Force
    }

    Set-Content -LiteralPath $glmMarker -Value $glmVersion -NoNewline
    Remove-Item -LiteralPath $glmExtract -Recurse -Force
}
else {
    Write-Host "GLM already present."
}

# ---------------------------------------------------------------------------
# Vulkan
#
# Each MSYS2 package's outer ucrt64 directory becomes .toolchain\vulkan.
# Everything below ucrt64 is preserved:
#
# ucrt64\bin     -> .toolchain\vulkan\bin
# ucrt64\include -> .toolchain\vulkan\include
# ucrt64\lib     -> .toolchain\vulkan\lib
# ---------------------------------------------------------------------------

$vulkanVersion = "msys2-ucrt64-vulkan-1.4.350.1-shaderc-2026.2"
$vulkanMarker = Join-Path $vulkan ".vulkan-version"

$vulkanPackages = @(
    @{
        Name = "vulkan-headers"
        File = "mingw-w64-ucrt-x86_64-vulkan-headers-1~1.4.350.1-1-any.pkg.tar.zst"
        Sha256 = "0E2CFDA1CB9D936255AA7B6947F837285EFB588C791F3B8476ABA1D745268AA5"
    },
    @{
        Name = "vulkan-loader"
        File = "mingw-w64-ucrt-x86_64-vulkan-loader-1~1.4.350.1-1-any.pkg.tar.zst"
        Sha256 = "48F2B2857C553CF564FDB0EBA709AE00879CD7193195E0F17C29CA9BEB424863"
    },
    @{
        Name = "vma"
        File = "mingw-w64-ucrt-x86_64-vulkan-memory-allocator-3.4.0-1-any.pkg.tar.zst"
        Sha256 = "44C81162337915AFECDA3185F66FD98268A54454580B2717080C9DFEF57FE781"
    },
    @{
        Name = "spirv-tools"
        File = "mingw-w64-ucrt-x86_64-spirv-tools-3~1.4.350.1-1-any.pkg.tar.zst"
        Sha256 = "AB9B477B15E826148F616C4E7339A33C8A2DD7EF7B8BC81849C2F25EBB2076E8"
    },
    @{
        Name = "glslang"
        File = "mingw-w64-ucrt-x86_64-glslang-16.3.0-1-any.pkg.tar.zst"
        Sha256 = "5A90C8AE45B42B500FA03669AA1A94D154DCD13E0BB254F86DF004B903826C03"
    },
    @{
        Name = "shaderc"
        File = "mingw-w64-ucrt-x86_64-shaderc-2026.2-1-any.pkg.tar.zst"
        Sha256 = "30F0D6C1B62671BEEE32D2642A798DECDE2CFECB8BEDF524679D6A31518A600D"
    },
    @{
        Name = "vulkan-utility-libraries"
        File = "mingw-w64-ucrt-x86_64-vulkan-utility-libraries-1.4.350.1-1-any.pkg.tar.zst"
        Sha256 = "FD73F99C6D97764F288CB3C45652BCE8F61564E4FD6FAD82E828E4A19D0CBF63"
    },
    @{
        Name = "vulkan-validation-layers"
        File = "mingw-w64-ucrt-x86_64-vulkan-validation-layers-1.4.350.1-1-any.pkg.tar.zst"
        Sha256 = "A98E3075BE7BC986F3BBD3D266DE605302659533B466278DE490A46E62D88AA9"
    }
)

$vulkanImportLibrary = Join-Path $vulkanLib "libvulkan-1.dll.a"
$vulkanMakefileAlias = Join-Path $vulkanLib "vulkan-1.lib"

$vulkanFiles = @(
    (Join-Path $vulkanBin "vulkan-1.dll"),
    (Join-Path $vulkanBin "glslc.exe"),
    (Join-Path $vulkanBin "libshaderc_shared.dll"),
    (Join-Path $vulkanBin "glslang.dll"),
    (Join-Path $vulkanBin "SPIRV.dll"),
    (Join-Path $vulkanBin "libglslang-default-resource-limits.dll"),
    (Join-Path $vulkanBin "libSPIRV-Tools-shared.dll"),
    (Join-Path $vulkanBin "VkLayer_khronos_validation.dll"),
    (Join-Path $vulkanBin "VkLayer_khronos_validation.json"),

    (Join-Path $vulkanInclude "vulkan\vulkan.hpp"),
    (Join-Path $vulkanInclude "vk_video\vulkan_video_codecs_common.h"),
    (Join-Path $vulkanInclude "vma\vk_mem_alloc.h"),

    $vulkanImportLibrary,
    $vulkanMakefileAlias
)

$vulkanReady = (Test-AllFiles $vulkanFiles) -and
    (Test-Path -LiteralPath $vulkanMarker -PathType Leaf) -and
    ((Get-Content -LiteralPath $vulkanMarker -Raw).Trim() -eq $vulkanVersion)

if (-not $vulkanReady) {
    Remove-Item -LiteralPath $vulkan -Recurse -Force -ErrorAction SilentlyContinue
    New-Item -ItemType Directory -Force -Path $vulkan | Out-Null

    foreach ($package in $vulkanPackages) {
        Install-UcrtPackage $package $vulkan
    }

    if (-not (Test-Path -LiteralPath $vulkanImportLibrary -PathType Leaf)) {
        throw "Vulkan import library was not installed: $vulkanImportLibrary"
    }

    # Compatibility copy for the current Makefile check:
    #
    # $(VULKAN_SDK)/lib/vulkan-1.lib
    #
    # The actual -lvulkan-1 link still resolves:
    #
    # $(VULKAN_SDK)/lib/libvulkan-1.dll.a

    Copy-Item `
        -LiteralPath $vulkanImportLibrary `
        -Destination $vulkanMakefileAlias `
        -Force

    $vulkanPeFiles = @(
        Get-ChildItem `
            -LiteralPath $vulkanBin `
            -File |
            Where-Object {
                $_.Extension -in @(".exe", ".dll")
            } |
            ForEach-Object {
                $_.FullName
            }
    )

    # Copies any required GCC/libstdc++/winpthread runtime DLLs from the
    # UCRT64 prefix into Vulkan's bin folder as well.

    Copy-PeDependencies `
        (Join-Path $ucrtBin "objdump.exe") `
        $vulkanPeFiles `
        @($vulkanBin, $ucrtBin) `
        $vulkanBin

    Set-Content -LiteralPath $vulkanMarker -Value $vulkanVersion -NoNewline
}
else {
    Write-Host "Vulkan toolchain already present."
}

# ---------------------------------------------------------------------------
# Verification
# ---------------------------------------------------------------------------

$requiredFiles = @(
    (Join-Path $ucrtBin "g++.exe"),
    (Join-Path $ucrtBin "gcc.exe"),
    (Join-Path $ucrtBin "cpp.exe"),
    (Join-Path $ucrtBin "as.exe"),
    (Join-Path $ucrtBin "ld.exe"),
    (Join-Path $ucrtBin "ar.exe"),
    (Join-Path $ucrtBin "ranlib.exe"),
    (Join-Path $ucrtBin "dlltool.exe"),
    (Join-Path $ucrtBin "windres.exe"),
    (Join-Path $ucrtBin "mingw32-make.exe"),
    (Join-Path $ucrtBin "make.exe"),
    (Join-Path $ucrtBin "objdump.exe"),

    (Join-Path $ucrtBin "libgcc_s_seh-1.dll"),
    (Join-Path $ucrtBin "libstdc++-6.dll"),
    (Join-Path $ucrtBin "libwinpthread-1.dll"),

    (Join-Path $ucrtBin "glfw3.dll"),
    (Join-Path $ucrtInclude "GLFW\glfw3.h"),
    (Join-Path $ucrtInclude "GLFW\glfw3native.h"),
    (Join-Path $ucrtInclude "glm\glm.hpp"),
    (Join-Path $ucrtLib "libglfw3.a"),
    (Join-Path $ucrtLib "libglfw3.dll.a"),

    (Join-Path $ucrtTargetLib "libgdi32.a"),
    (Join-Path $ucrtTargetLib "libole32.a"),
    (Join-Path $ucrtTargetLib "libuuid.a"),
    (Join-Path $ucrtTargetLib "libdwmapi.a"),

    (Join-Path $vulkanBin "vulkan-1.dll"),
    (Join-Path $vulkanBin "glslc.exe"),
    (Join-Path $vulkanBin "libshaderc_shared.dll"),
    (Join-Path $vulkanBin "glslang.dll"),
    (Join-Path $vulkanBin "SPIRV.dll"),
    (Join-Path $vulkanBin "libglslang-default-resource-limits.dll"),
    (Join-Path $vulkanBin "libSPIRV-Tools-shared.dll"),
    (Join-Path $vulkanBin "VkLayer_khronos_validation.dll"),
    (Join-Path $vulkanBin "VkLayer_khronos_validation.json"),

    (Join-Path $vulkanInclude "vulkan\vulkan.hpp"),
    (Join-Path $vulkanInclude "vk_video\vulkan_video_codecs_common.h"),
    (Join-Path $vulkanInclude "vma\vk_mem_alloc.h"),

    (Join-Path $vulkanLib "libvulkan-1.dll.a"),
    (Join-Path $vulkanLib "vulkan-1.lib")
)

foreach ($file in $requiredFiles) {
    if (-not (Test-Path -LiteralPath $file -PathType Leaf)) {
        throw "Required dependency is missing: $file"
    }
}

foreach ($name in @(
    "cc1.exe",
    "cc1plus.exe",
    "collect2.exe"
)) {
    $match = Get-ChildItem `
        -LiteralPath (Join-Path $ucrt64 "libexec") `
        -Recurse `
        -File `
        -Filter $name `
        -ErrorAction SilentlyContinue |
        Select-Object -First 1

    if ($null -eq $match) {
        throw "Required compiler component is missing: $name"
    }
}

$oldPath = $env:PATH
$env:PATH = "$ucrtBin;$vulkanBin;$oldPath"

try {
    Write-Host "Verifying executables..."

    & (Join-Path $ucrtBin "g++.exe") --version |
        Select-Object -First 1

    if ($LASTEXITCODE -ne 0) {
        throw "g++.exe verification failed."
    }

    & (Join-Path $ucrtBin "make.exe") --version |
        Select-Object -First 1

    if ($LASTEXITCODE -ne 0) {
        throw "make.exe verification failed."
    }

    & (Join-Path $vulkanBin "glslc.exe") --version |
        Select-Object -First 1

    if ($LASTEXITCODE -ne 0) {
        throw "glslc.exe verification failed."
    }

    Write-Host "Verifying the existing linker flags..."

    $linkTestSource = Join-Path $cache "link-test.cpp"
    $staticLinkTest = Join-Path $cache "link-test-static.exe"
    $dynamicLinkTest = Join-Path $cache "link-test-dynamic.exe"

    Set-Content `
        -LiteralPath $linkTestSource `
        -Value "int main() { return 0; }" `
        -NoNewline

    & (Join-Path $ucrtBin "g++.exe") `
        $linkTestSource `
        -o $staticLinkTest `
        "-L$vulkanLib" `
        -static `
        -lvulkan-1 `
        -lglfw3 `
        -lgdi32 `
        -lole32 `
        -luuid `
        -ldwmapi

    if ($LASTEXITCODE -ne 0) {
        throw "Static linker-flag verification failed."
    }

    & (Join-Path $ucrtBin "g++.exe") `
        $linkTestSource `
        -o $dynamicLinkTest `
        "-L$vulkanLib" `
        -lvulkan-1 `
        -lglfw3 `
        -lgdi32 `
        -lole32 `
        -luuid `
        -ldwmapi

    if ($LASTEXITCODE -ne 0) {
        throw "Dynamic linker-flag verification failed."
    }

    Remove-Item `
        -LiteralPath $linkTestSource, $staticLinkTest, $dynamicLinkTest `
        -Force
}
finally {
    $env:PATH = $oldPath
}

Remove-Item `
    -LiteralPath $cache `
    -Recurse `
    -Force `
    -ErrorAction SilentlyContinue

Write-Host ""
Write-Host "Setup completed."
