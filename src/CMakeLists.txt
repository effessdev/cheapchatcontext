# CMakeList.txt : CMake project for CheapChatContext, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (ccc "main.cpp"  "ProjectScanner.h" "ProjectScanner.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ccc PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
