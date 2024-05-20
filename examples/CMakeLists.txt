add_subdirectory(extlibs)

# ----------------
# External dependencies for examples.
# ----------------

find_package(imgui CONFIG REQUIRED)
find_package(Stb REQUIRED)

# ----------------
# Example targets.
# ----------------

add_subdirectory(00-compute-device)
add_subdirectory(01-headless-triangle)
add_subdirectory(02-headless-msaa-triangle)
add_subdirectory(03-cube-in-flight)
add_subdirectory(04-imgui)