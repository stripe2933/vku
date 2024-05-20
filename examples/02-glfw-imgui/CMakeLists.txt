add_executable(vku-examples-02-glfw-imgui main.cpp ../impl.cpp)
target_compile_features(vku-examples-02-glfw-imgui PRIVATE cxx_std_23)
target_link_libraries(vku-examples-02-glfw-imgui PRIVATE
    imgui::imgui
    vku-examples-extlibs
    vku
)