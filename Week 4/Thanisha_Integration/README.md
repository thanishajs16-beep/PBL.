# Deadlock Simulation System – Integration Work

## 1. Project Overview
This folder contains the integration work for the Week 4 Deadlock Simulation System project. The system demonstrates resource allocation, deadlock avoidance, deadlock detection, and recovery through a graphical user interface.

## 2. Integration Work
The integration work includes:
- Testing Banker's Algorithm and safe-sequence generation.
- Testing deadlock detection and identification of deadlocked processes.
- Testing Resource Allocation Graph generation.
- Testing deadlock recovery and resource release.
- Verifying resource utilization and system status.
- Supporting the build and testing process.

## 3. Integration Test Files
- `test_core.c` – Tests Banker's Algorithm and core resource-management behavior.
- `test_detection.c` – Tests deadlock detection, Resource Allocation Graph generation, and recovery.
- `build.sh` – Builds the GUI application and runs the core integration tests.

## 4. Main Application Features
The integrated application provides:
- Process and resource creation.
- Allocation, maximum, need, and request matrices.
- Banker's Algorithm and safe-sequence display.
- SAFE and UNSAFE state reporting.
- Deadlock detection and recovery.
- Resource Allocation Graph visualization.
- Resource utilization display.
- System Status and Refresh Display controls.

## 5. Build and Run
Run these commands from the main Week 4 project directory in Ubuntu or WSL.

Build the GUI:
```bash
bash build.sh
```

Launch the application:
```bash
./deadlock_simulator
```

Run the core integration tests:
```bash
bash build.sh test
```

GTK 3 development libraries and the GCC compiler are required to build the GUI.

## 6. Testing Summary
The core integration tests passed. The GUI was built and launched successfully, and the tested scenarios included safe and unsafe states, deadlock detection, recovery, resource requests, and resource utilization.

## 7. Purpose
This integration work helps bring the individual components together into a working Deadlock Simulation System and supports verification of the main project requirements.
