# Gaussian Splat Viewer

This is an application that allows viewing gaussian splats (.ply only) in an easy way.
The renderer is made with Vulkan and GLSL.
Currently, it runs ONLY on Windows 10 or later with a GPU that has Vulkan support. 
Most GPUs including integrated ones like Intel UHD Graphics should have this by default.

<img width="510" height="404" alt="Image" src="https://github.com/user-attachments/assets/360a318e-c7a8-4382-b2f5-a0f06f090f81" />

<img width="510" height="404" alt="Image" src="https://github.com/user-attachments/assets/df7cc5e4-6ee7-4c58-9b32-bf04d756a5d5" />

### Requirements (for running)

1. Windows 10 or later (required)
2. GPU with Vulkan support (required)
3. Quad-core processor (recommended)
4. 8 GB RAM (recommended)

***Note:***<br>
Since there is no frame rate limiter, the GPU would utilise all available resources (unless externally restricted).
Close the viewer when not in use to reduce power consumption and heat.

### Troubleshooting (Running)

The executable does not contain a Microsoft certificate, so it may be flagged as suspicious.
If you do not trust the executable in release, you can always just build the project on your own.
(Refer to Code Installation below)

If the viewer feels laggy, shrink the size of the window.
This is to reduce the per-pixel GPU work, so you will get a smoother experience

## Controls

The camera controls are similar to most 3D editors.

- **Right Mouse Button (RMB):** Look around
- **RMB + W:** Move forward
- **RMB + A:** Move left
- **RMB + S:** Move backward
- **RMB + D:** Move right
- **RMB + E:** Move up
- **RMB + Q:** Move down

Make sure to hold down RMB when moving.

## Examples

I have compiled some splats in this 
[google drive folder](https://drive.google.com/drive/folders/1D8GRehKaSA6uY1TZnMk8U7GyuP6YOJhg?usp=sharing).

***Note:***<br>
I do not own any of these splats, the original creators are as listed:

classroom.ply (CC BY 4.0)<br>
Title: Classroom of Class 6 Grade 9, China<br>
Author: Hite Spive (@hite404)<br>
Link: https://superspl.at/scene/712d5b78

cat.ply (CC BY 4.0)<br>
Title: Kitty<br>
Author: Andrej Kurcik (@andyone)<br>
Link: https://superspl.at/scene/79db08c0

minecraft.ply (CC BY 4.0)<br>
Title: Minecraft Small Village (VR Ready ✅)<br>
Author: Rodrigo C. (@kikooooo2)<br>
Link: https://superspl.at/scene/c6bba01c

lego.ply (CC BY 4.0)<br>
Title: LEGO® 42056 Porsche 911 GT3 RS (LDraw)<br>
Author: Renderbricks (@renderbricks)<br>
Link: https://superspl.at/scene/98c4ad8d

## Code Installation 

***Note:***<br>
This is only if you want to compile and run the code on your own. 
If you only wanted to try the app, you can download it from release.
The executable is inside the attached zip.

### Hardware Requirements (for building)

1. GPU with Vulkan support (required)
2. At least 16GB RAM (required)
3. At least 1GB Storage (required)
4. Quad-core processor or better (recommended)

### Software Requirements (for building)

1) Windows 10 or later.
2) Visual Studio Code (not to be confused with Visual Studio)

Unfortunately, this is a requirement to run and compile the code as I have set some local environment variables including:
(.vscode/settings.json)

- `path to UCRT64 binaries`
- `VULKAN_SDK`
- `VK_ADD_LAYER_PATH`

Do note that the build will fail on other editors.
(Unless you have configured the environment variables yourself) 

Even if you have C++ and Vulkan installed, this still needs to be run.
This is to ensure that all dependencies are the same as mine to prevent any unexpected errors.

No other prior installations are needed.

### Steps:
1. Download and extract the folder.
2. Open the folder in VS Code
3. Open a terminal
4. run:

```powershell
./setup.bat
```

5. After `Setup Completed` is printed, run 

```powershell
make
```
This should build and run the application in debug mode meaning:

- Logging is enabled
- Vulkan validation layer is enabled
- Shaders are not baked into the executable

*(for other make commands refer to CommandsList.txt)*

### Troubleshooting (Building)

In the case where setup fails you could try rerunning 

```powershell
./setup.bat
```

It would continue from where it left off, 
so you do not need to worry about it starting all over again.

If it still fails, wait an hour before trying again. 
Most likely, one of the mirror sites used to download the dependencies may be temporarily unavailable.

## Acknowledgements

The implementation of this follows the forward rendering as described in the original 3DGS paper. (arXiv:2308.04079 [cs.GR])

The code is written by me with some AI guidance by GPT 5.5 (and later parts by GPT 5.6 Sol) in parts including: 
understanding Vulkan boilerplates and Win32 API, 
understanding the implementations described by 3DGS,
finding vulnerabilities in memory management,
and giving general feedback.
