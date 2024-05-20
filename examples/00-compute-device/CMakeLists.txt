add_executable(vku-examples-00-compute-device main.cpp ../impl.cpp)
target_compile_features(vku-examples-00-compute-device PRIVATE cxx_std_23)
target_link_libraries(vku-examples-00-compute-device PRIVATE
    vku-examples-extlibs
    vku
)
target_compile_definitions(vku-examples-00-compute-device PRIVATE
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
)

# Shaders are automatically compiled to SPIR-V binaries, and their locations are defined as compile definitions
# (<target-name>_COMPILED_SHADER_DIR). Also they are embed as compile-time constant if release build enabled.
target_compile_shaders(vku-examples-00-compute-device
    shaders/multiply.comp
)