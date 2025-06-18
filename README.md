# Graph Editor

## Overview

Graph Editor is a C++ application built with the Qt framework that allows users to create, edit, and visualize directed weighted graphs. It includes an interactive step-by-step visualization of Dijkstra’s shortest path algorithm.

![image](https://github.com/user-attachments/assets/45c27db9-385d-4815-9b22-af23a5b38aa9)

## Key Features

- Intuitive interface for creating, selecting, moving, and deleting graph vertices.
- Add directed edges with custom non-negative weights (integer or floating-point).
- Support for arbitrary graph sizes and structures.
- Interactive selection of the start vertex to run Dijkstra’s algorithm.
- Real-time, stepwise visualization of algorithm execution:
  - Highlights current, start, and visited vertices.
  - Displays updated weights and processed edges dynamically.
  - Final animation marking algorithm completion.

## How It Works

- Vertices can be freely positioned on a 2D canvas.
- Edges connect vertices, with weights manually entered by the user.
- The algorithm runs from a selected starting vertex, calculating shortest paths to all others.
- The visualization provides clear feedback on the algorithm’s progress and results.

## Dijkstra Algorithm and Extensibility

The program includes an implementation of Dijkstra's algorithm for finding the shortest paths from a selected start vertex to all other vertices in the graph. This implementation is encapsulated in a dedicated class with static methods, allowing straightforward invocation without creating objects.

![image](https://github.com/user-attachments/assets/743e0a1f-2309-4dd5-bc11-28a91c93b75f)

### Features of the Dijkstra Implementation:
- Initializes all vertex weights and updates them based on edge weights.
- Selects the next vertex with the minimum tentative distance during each iteration.
- Updates neighboring vertices’ weights through edge relaxation.
- Provides step-by-step visualization of the algorithm’s progress, highlighting the current vertex, updated paths, and final results.

### Extensibility for Other Algorithms

The program architecture is designed to support easy integration of additional graph algorithms and functionalities in the future. Thanks to modular design and separation of concerns, new algorithms can be implemented as separate classes or modules and connected to the existing graph data structures and visualization system with minimal effort.

This makes the project a flexible foundation for further development in graph analysis, visualization, and algorithm demonstration.


## Technologies Used

- C++ programming language  
- Qt framework for GUI and event handling

