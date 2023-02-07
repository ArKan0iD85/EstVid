# EstVid - Video Stabilization
Basic implementation of a video stabilizer using OpenCV, recording the stabilized video and also side-by-side comparison.

Also provides a real-time implementation, but still in early stage and non-functional.

### Compilation information

- Used IDE: Microsoft Visual Studio Community 2022 (64 bits) v17.4.4
- Platform toolset: Visual Studio 2022 (MSVC v143)
- Windows SDK version: v10.0
- External libraries: OpenCV v4.5.5

### How to Run

- Clone the repository and compile the program using the above-mentioned information
- Run the "EstVid.exe" program created in the “\EstVid\x64\Release\” folder
- Follow the steps shown on the screen

### Warning

- This program considers the existence of the folders "input", "output" and "stats" where the EstVid solution is placed
- Real-time stabilization mode is still under development and does not correctly display the stabilized video.
- Windows only.