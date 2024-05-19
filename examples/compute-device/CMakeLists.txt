add_executable(vku-examples-compute-device main.cpp)
target_compile_features(vku-examples-compute-device PRIVATE cxx_std_23)
target_link_libraries(vku-examples-compute-device PRIVATE
    vku-examples-extlibs
    vku
)