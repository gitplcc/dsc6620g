set(${PROJECT_NAME}_SOURCES
    main.cpp
    dlink_camera.hpp
    dlink_camera.cpp
)

set(IMGUI_SOURCES
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp
)

if(WIN32)
    add_executable(${PROJECT_NAME} WIN32)
else()
    add_executable(${PROJECT_NAME})
endif()
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)
target_sources(${PROJECT_NAME}
    PRIVATE
        ${${PROJECT_NAME}_SOURCES}
        ${IMGUI_SOURCES}
)
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        SDL3::SDL3
        libcurl
        ffmpeg::avformat
        ffmpeg::avcodec
        ffmpeg::avutil
        ffmpeg::swscale
)
