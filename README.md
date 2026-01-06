2D & 3D Solar System Graphics Project

This project is an interactive 2D and 3D graphical simulation of the Solar System, developed as part of a computer graphics course. It visualizes planetary motion, orbits, and spatial relationships between celestial bodies, allowing users to freely explore the system in both 2D and 3D modes using keyboard and mouse controls.

The simulation focuses on real-time rendering, camera movement, and user interaction, providing an intuitive way to observe planetary dynamics and graphical representations.

Features
Dual Representation (2D & 3D)

The Solar System can be visualized in both 2D and 3D modes.

Seamless switching between representations for different perspectives and analysis.

Planetary Motion

Planets orbit around the Sun with continuous animation.

Orbital speed can be increased or decreased dynamically.

Planet motion can be paused or resumed at any time.

Camera & Navigation

Free camera movement using keyboard and mouse.

Smooth rotation, zooming, and exploration of the scene.

Enables close inspection of individual planets or the entire system.

Orbit Visualization

Planetary orbits can be toggled on or off.

Helps clearly visualize orbital paths and relative distances.

Rendering Modes

Multiple visualization modes for educational and debugging purposes:

Textured view (full surface textures)

Vertex view

Wireframe / edge view

Real-Time Interaction

All changes (camera, speed, view modes) happen instantly without restarting the simulation.

Controls
Movement & Camera

W / A / S / D – Move the camera

Mouse movement – Look around

Mouse left press and hold - Focused planet trivia and details.

Mouse scroll – Zoom in / zoom out

Simulation Control

P – Pause / resume planet movement

+ – Increase planetary speed

- – Decrease planetary speed

Visualization Options

O – Toggle orbit visibility

1 – Full textured rendering

2 – Vertex view

3 – Edge / wireframe view

Technologies Used

Programming Language: C++

Graphics API: OpenGL

Libraries: GLFW / GLAD (or equivalent, depending on your setup)

Shaders: GLSL (vertex and fragment shaders)

Installation and Usage
Clone the Repository
git clone https://github.com/yourusername/Solar-System-2D-3D.git

Build and Run

Make sure OpenGL and required libraries are installed.

Build the project using your preferred compiler or IDE.

Run the executable to start the simulation.

Educational Purpose

This project demonstrates:

Practical use of OpenGL rendering pipelines

2D and 3D transformations

Camera systems and user interaction

Real-time animation and visualization techniques
