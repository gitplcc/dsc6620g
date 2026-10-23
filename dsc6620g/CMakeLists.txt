# --- 1. Definición del Ejecutable y Configuración de Fuentes ---

# Estructura de archivos de tu proyecto
if(WIN32)
    add_executable(${PROJECT_NAME} WIN32)
else()
    add_executable(${PROJECT_NAME})
endif()
set(${PROJECT_NAME}_SOURCES
    main.cpp
    dlink_camera.cpp
    dlink_camera.hpp
)
target_sources(${PROJECT_NAME}
    PRIVATE
        ${${PROJECT_NAME}_SOURCES}
)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)

# --- 2. Inclusión de Directorios y Enlazado de Librerías ---

# Enlazar las dependencias descargadas por CPM
target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL3::SDL3
    libcurl
)

# Vincular los archivos fuente de ImGui directamente a nuestro proyecto (estilo amigable de ImGui)
target_include_directories(${PROJECT_NAME} PRIVATE ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_sources(${PROJECT_NAME} PRIVATE
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp
)

# Enlazar los componentes de FFmpeg encontrados por PkgConfig
target_include_directories(${PROJECT_NAME} PRIVATE
    ${AVFORMAT_INCLUDE_DIRS}
    ${AVCODEC_INCLUDE_DIRS}
    ${AVUTIL_INCLUDE_DIRS}
    ${SWSCALE_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${AVFORMAT_LIBRARIES}
    ${AVCODEC_LIBRARIES}
    ${AVUTIL_LIBRARIES}
    ${SWSCALE_LIBRARIES}
)
