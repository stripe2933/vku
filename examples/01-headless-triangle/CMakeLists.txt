add_executable(vku-examples-01-headless-triangle main.cpp ../impl.cpp)
target_compile_features(vku-examples-01-headless-triangle PRIVATE cxx_std_23)
target_include_directories(vku-examples-01-headless-triangle PRIVATE ${Stb_INCLUDE_DIR})
target_link_libraries(vku-examples-01-headless-triangle PRIVATE
    vku-examples-extlibs
    vku
)
target_compile_definitions(vku-examples-01-headless-triangle PRIVATE
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
)

target_compile_shaders(vku-examples-01-headless-triangle
    shaders/triangle.vert shaders/triangle.frag
)