# Pevi - 3D Code Editor

Pevi is an innovative code editor that operates in 3D space. Unlike traditional text editors that are confined to 2D windows, Pevi allows code to be visualized and manipulated in a three-dimensional environment.

## Key Features

- **3D Workspace**: Navigate through your code in a fully 3D environment
- **Floating Panes**: Code is displayed on panes that float in 3D space
- **Chunk-Based Editing**: Different panes can represent different chunks or sections of files
- **Spatial Organization**: Organize your code spatially to visualize relationships between components
- **Immersive Editing**: Experience a new way of interacting with your codebase

Pevi aims to provide a unique perspective on code editing by leveraging spatial relationships to enhance understanding of complex codebases.

## Current Implementation Status

### Working Features
- 3D camera navigation with mouse and keyboard controls (WASD, arrow keys, and mouse)
- Basic phantom (floating code pane) rendering in 3D space
- Text rendering with SDF fonts for crisp display at any distance
- File loading and basic text buffer management using linked ring data structure
- Command mode with simple commands (e.g., 'q' to quit)
- Basic text editing capabilities (insert/delete characters)
- Mode switching between free navigation, edit, and command modes
- Cursor positioning and text manipulation
- Hover detection and interaction with text elements

### Interaction Controls
- **Navigation Mode**: 
  - WASD/Arrow keys: Move camera position
  - Mouse: Look around
  - Space/A/D: Move sideways
  - Enter/Backspace: Move up/down
  - E key: Switch to edit mode
  - Colon (:): Enter command mode

- **Edit Mode**:
  - Type to insert text at cursor position
  - Backspace to delete characters
  - ESC: Return to navigation mode

- **Command Mode**:
  - Type commands (e.g., 'q' to quit)
  - Enter: Execute command
  - ESC: Return to navigation mode

### In Development
- Multiple phantom support for organizing code chunks
- Advanced text selection and manipulation
- Syntax highlighting
- Project management features
- Enhanced navigation between code sections
- Improved UI for editing operations
- File browser and project structure visualization
- Undo/redo functionality

### Technical Foundation
- Built with Raylib for 3D rendering and input handling
- Custom text buffer implementation using linked rings
- SDF font rendering for high-quality text at various distances
- Camera system with multiple viewing modes

## Building and Running

### Prerequisites
- CMake (version 3.10 or higher)
- C compiler with C11 support (GCC, Clang)
- Raylib dependencies

### Build Instructions

1. Clone the repository:
   ```bash
   git clone https://github.com/fefa4ka/pevi.git
   cd pevi
   ```

2. Create a build directory:
   ```bash
   mkdir build && cd build
   ```

3. Configure with CMake:
   ```bash
   cmake ..
   ```

4. Build the project:
   ```bash
   cmake --build .
   ```

5. Run the editor:
   ```bash
   ./src/editor
   ```

### Development Build

To build with debug symbols and tests:
```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON ..
cmake --build .
```

The project uses CMake for building. See the CMakeLists.txt for details on dependencies and build configuration.

