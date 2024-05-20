add_executable(vku-examples-04-imgui main.cpp ../impl.cpp)
target_compile_features(vku-examples-04-imgui PRIVATE cxx_std_23)
target_link_libraries(vku-examples-04-imgui PRIVATE
    imgui::imgui
    vku-examples-extlibs
    vku
)
target_compile_definitions(vku-examples-04-imgui PRIVATE
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
)