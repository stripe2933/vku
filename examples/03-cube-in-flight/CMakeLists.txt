add_executable(vku-examples-03-cube-in-flight main.cpp ../impl.cpp)
target_compile_features(vku-examples-03-cube-in-flight PRIVATE cxx_std_23)
target_link_libraries(vku-examples-03-cube-in-flight PRIVATE
    vku-examples-extlibs
    vku
)
target_compile_definitions(vku-examples-03-cube-in-flight PRIVATE
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
)

target_compile_shaders(vku-examples-03-cube-in-flight
    shaders/mesh.vert shaders/mesh.frag
)