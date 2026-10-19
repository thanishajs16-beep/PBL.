 Week 4 – Deadlock Simulation GUI

This folder contains the GUI part of the Interactive Deadlock Simulation System. The interface is written in C using GTK 3, with Cairo used to draw the Resource Allocation Graph.

The GUI lets users enter process and resource information, view the system's current data, and see the results returned by the deadlock and resource-management functions.

Files:

- `main.c` – Initializes the deadlock system and launches the application.
- `gui.c` – Contains the main window, input dialogs, system displays, and Resource Allocation Graph drawing code.

 What the GUI displays:

- **System Matrices:** Process details, total and available resources, Allocation, Maximum, Need, and Request matrices.
- **Algorithm Results:** Results from Banker's Algorithm, including safety status and safe sequence, plus deadlock detection results.
- **Resource Allocation Graph:** A visual view of process and resource nodes and their relationships.
- **Resource Utilization:** Resource utilization information based on the current system data.
- **System Status:** A summary of the current safety and deadlock state.

The GUI also provides forms for entering processes and resource types and updating allocation, maximum, and request values.

 Technologies:

- C
- GTK 3
- Cairo

My part:

Student 1 – GUI and Display. My work covers the application window, input forms, display tabs, matrix and result presentation, and the visual Resource Allocation Graph. The algorithm logic is supplied through the project's shared backend and integration.

NOTE:

`gui.c` depends on the shared `deadlock.h` interface and the backend implementation used by the full project. The GUI files need to be built together with those project dependencies to run the complete application.
