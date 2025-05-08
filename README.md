# Diggable-Terrain
(2025) Tech demo of deformable meshes using Unreal Engine 5's geometry script and the marching cubes algorithm.

## Technologies/Techniques Used:
- Unreal Engine 5 geometry script and dynamic meshes
- Realtime mesh deformation with collision using custom generated primitives
- Implementation of marching cubes algorithm to determine vertices and triangles from point grid

## Features:
- Vertices shared between triangles for performance
- Distance calculations only performed on meshes overlapping with the carve tool
- Mesh supports complex collision for line tracing and simple collision for physics using generated primitive cubes
- Exposed parameters for grid size, world size, the option to generate UV coordinates and normals, UV scale, and more
- Ability to both subtract and add to the mesh
- Meshes can be placed next to each other and will join seamlessly

## Gallery:
<img src="https://github.com/user-attachments/assets/31adc427-a849-4419-8629-ea821eb940df" width="311" height="174" style="display:none">
<img src="https://github.com/user-attachments/assets/946f6256-e546-4fed-aa17-0bc7b4612b6e" width="311" height="174" style="display:none">
<img src="https://github.com/user-attachments/assets/d2bdfd04-5047-4eb4-9a2c-1b7a6ecdd525" width="311" height="174" style="display:none">
<img src="https://github.com/user-attachments/assets/d65f589f-1575-49f7-b424-c78f2b0154a8" width="311" height="174" style="display:none">

## Acknowledgements:
The function 'CalculateUVsAndNormal' was adapted from Tristan367's StackOverflow answer: https://stackoverflow.com/a/74635225
