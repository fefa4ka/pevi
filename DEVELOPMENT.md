# Pevi Development Plan

This document outlines the immediate development priorities for Pevi, the 3D code editor.

## Immediate Development Goals

### 1. Multiple Phantom Support
**Goal**: Allow multiple code panes (phantoms) to exist in the 3D space simultaneously.

**Tasks**:
- Create a phantom manager to track and handle multiple phantoms
- Implement phantom creation/deletion commands
- Add navigation between phantoms
- Enable repositioning of phantoms in 3D space
- Implement phantom selection and focus handling

**Technical approach**:
- Create a phantom collection in the Pevi_t structure
- Add phantom ID generation and tracking
- Extend input handling to manage focus between phantoms
- Implement commands for phantom management (new, close, focus)

### 2. Syntax Highlighting
**Goal**: Add syntax highlighting to improve code readability.

**Tasks**:
- Implement a lexer for common programming languages
- Create a color scheme system
- Extend the text rendering system to support colored text
- Add language detection based on file extension
- Implement token-based rendering in phantoms

**Technical approach**:
- Create a simple lexer that identifies tokens (keywords, strings, comments, etc.)
- Extend the symbol_draw function to accept color information
- Add color configuration to the font settings
- Implement token caching for performance

### 3. Project Management
**Goal**: Add project management capabilities to handle multiple files.

**Tasks**:
- Implement a project configuration system
- Create a file browser for navigating project files
- Add project-wide search functionality
- Implement file opening/closing within a project context
- Add project visualization in 3D space

**Technical approach**:
- Create a project structure to track files and directories
- Implement file system traversal for project loading
- Add a UI component for file browsing
- Create a 3D visualization of project structure
- Extend phantom system to represent files within a project

## Implementation Priority

1. Multiple Phantom Support - This is the foundation for a true multi-file editor
2. Syntax Highlighting - This will significantly improve usability and readability
3. Project Management - This will make Pevi practical for real-world development

## Technical Debt to Address

- [x] Refactor the input handling system to better separate concerns
- Improve memory management for buffers and phantoms
- Add proper error handling throughout the codebase
- Create a more robust UI framework for editor components
