# Install script for directory: D:/Creat/GAME/TowerClash_with_CNN/external/DirectXTK-may2026

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/TowerClash_with_CNN")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/Creat/GAME/TowerClash_with_CNN/build/lib/DirectXTK.lib")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/directxtk/DirectXTK-targets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/directxtk/DirectXTK-targets.cmake"
         "D:/Creat/GAME/TowerClash_with_CNN/build/external/DirectXTK-may2026/CMakeFiles/Export/a11a99d19d8d3c8432b0fa94ef825414/DirectXTK-targets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/directxtk/DirectXTK-targets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/directxtk/DirectXTK-targets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/directxtk" TYPE FILE FILES "D:/Creat/GAME/TowerClash_with_CNN/build/external/DirectXTK-may2026/CMakeFiles/Export/a11a99d19d8d3c8432b0fa94ef825414/DirectXTK-targets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/directxtk" TYPE FILE FILES "D:/Creat/GAME/TowerClash_with_CNN/build/external/DirectXTK-may2026/CMakeFiles/Export/a11a99d19d8d3c8432b0fa94ef825414/DirectXTK-targets-debug.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/directxtk" TYPE FILE FILES
    "D:/Creat/GAME/TowerClash_with_CNN/external/DirectXTK-may2026/Inc/BufferHelpers.h"
    "D:/Creat/GAME/TowerClash_with_CNN/external/DirectXTK-may2026/Inc/CommonStates.h"
    "D:/Creat/GAME/TowerClash_with_CNN/external/DirectXTK-may2026/Inc/DDSTextureLoader.h"
    "D:/Creat/GAME/TowerClash_with_CNN/external/DirectXTK-may2026/Inc/DirectXHelpers.h"
    "D:/Creat/GAME/TowerClash_with_CNN/external/DirectXTK-may2026/Inc/Effects.h"
    "D:/Creat/GAME/TowerClash_with_CNN/external/DirectXTK-may2026/Inc/GeometricPrimitive.h"
    "D:/Creat/GAME/TowerClash_with_CNN/external/DirectXTK-may2026/Inc/GraphicsMemory.h"
    "D:/Creat/GAME/TowerClash_with_CNN/external/DirectXTK-may2026/Inc/Model.h"
    "D:/Creat/GAME/TowerClash_with_CNN/external/DirectXTK-may2026/Inc/PostProcess.h"
    "D:/Creat/GAME/TowerClash_with_CNN/external/DirectXTK-may2026/Inc/PrimitiveBatch.h"
    "D:/Creat/GAME/TowerClash_with_CNN/external/DirectXTK-may2026/Inc/ScreenGrab.h"
    "D:/Creat/GAME/TowerClash_with_CNN/external/DirectXTK-may2026/Inc/SpriteBatch.h"
    "D:/Creat/GAME/TowerClash_with_CNN/external/DirectXTK-may2026/Inc/SpriteFont.h"
    "D:/Creat/GAME/TowerClash_with_CNN/external/DirectXTK-may2026/Inc/VertexTypes.h"
    "D:/Creat/GAME/TowerClash_with_CNN/external/DirectXTK-may2026/Inc/WICTextureLoader.h"
    "D:/Creat/GAME/TowerClash_with_CNN/external/DirectXTK-may2026/Inc/SimpleMath.h"
    "D:/Creat/GAME/TowerClash_with_CNN/external/DirectXTK-may2026/Inc/SimpleMath.inl"
    "D:/Creat/GAME/TowerClash_with_CNN/external/DirectXTK-may2026/Inc/GamePad.h"
    "D:/Creat/GAME/TowerClash_with_CNN/external/DirectXTK-may2026/Inc/Keyboard.h"
    "D:/Creat/GAME/TowerClash_with_CNN/external/DirectXTK-may2026/Inc/Mouse.h"
    "D:/Creat/GAME/TowerClash_with_CNN/external/DirectXTK-may2026/Inc/Audio.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/directxtk" TYPE FILE FILES
    "D:/Creat/GAME/TowerClash_with_CNN/build/external/DirectXTK-may2026/directxtk-config.cmake"
    "D:/Creat/GAME/TowerClash_with_CNN/build/external/DirectXTK-may2026/directxtk-config-version.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "D:/Creat/GAME/TowerClash_with_CNN/build/external/DirectXTK-may2026/DirectXTK.pc")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "D:/Creat/GAME/TowerClash_with_CNN/build/external/DirectXTK-may2026/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
