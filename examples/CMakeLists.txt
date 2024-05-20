add_subdirectory(extlibs)

find_package(Stb REQUIRED)
find_package(imgui CONFIG REQUIRED)

add_subdirectory(00-compute-device)
add_subdirectory(01-headless-triangle)
add_subdirectory(02-glfw-imgui)