add_subdirectory(UITestApp)


add_library(UICore
        source/Slider.cpp
        source/Label.cpp)

target_include_directories(UICore
        PRIVATE include/UICore
        PUBLIC include)

target_link_libraries(UICore PRIVATE
        juce::juce_gui_basics)

target_compile_features(UICore PUBLIC cxx_std_17)