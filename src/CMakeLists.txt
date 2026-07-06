# Define a library 'scanner' using the source file. This allows modular code organization.
# scanner: The internal name you give this library. You use this name later to "link" it to other things.
# ProjectScanner.cpp: The source file(s) that belong to this library.
add_library(ccc_lib ProjectScanner.cpp)

# Define the main application executable from main.cpp
# ccc: The name of the final output file (e.g., ccc.exe).
# main.cpp: The entry point file containing your int main() function.
add_executable(ccc main.cpp)

# Link the executable to the 'scanner' library. 
# 'PRIVATE' means these dependencies are only used by the executable, not inherited by things that use the executable.
target_link_libraries(ccc PRIVATE ccc_lib)
