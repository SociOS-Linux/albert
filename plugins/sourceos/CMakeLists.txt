cmake_minimum_required(VERSION 3.19)

project(sourceos
    VERSION 0.1.0
    DESCRIPTION "SourceOS action bus plugin"
)

# In-tree build: FindAlbert.cmake in the build directory provides the macros.
# Standalone build: find_package(Albert) resolves from system install.
find_package(Albert REQUIRED)

# Minimal plugin: no UI config yet.
albert_plugin(
    QT Core
)
