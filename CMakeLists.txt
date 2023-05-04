cmake_minimum_required (VERSION 3.14)

project(Ray_Trace VERSION 1.0)

find_package(SFML 2.5 
  COMPONENTS 
    system window graphics network audio REQUIRED)

find_package(OpenGL)


set (DFLAGS 
-g 
-ggdb3 
-Wall 
-Werror 
-Wpedantic 
-Wextra 
-fsanitize=undefined,address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr
)

set (SFMLFLAGS
sfml-graphics 
sfml-window 
sfml-network 
sfml-audio
)

#add_library(IMGUILIB ${CMAKE_CURRENT_SOURCE_DIR}/ImGui/include ${CMAKE_CURRENT_SOURCE_DIR}/ImGui/source ${IMGUI_SOURCES})
#target_include_directories(IMGUILIB PUBLIC ${IMGUI_INCLUDE_DIR})
#target_link_libraries(IMGUILIB imgui)
#set_target_properties(IMGUILIB PROPERTIES LINKER_LANGUAGE CXX)
add_executable (${PROJECT_NAME} src/main.cpp include/ImGui/imgui.cpp include/ImGui/imgui_draw.cpp include/ImGui/imgui_tables.cpp include/ImGui/imgui_widgets.cpp include/ImGui/imgui-SFML.cpp )
#target_link_options(${PROJECT_NAME} PRIVATE "${DFLAGS}")
#target_compile_options(${PROJECT_NAME} PRIVATE "${SFMLFLAGS}")
target_include_directories (${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${SFML_INCLUDE_DIR})
target_link_libraries (${PROJECT_NAME} PRIVATE ${SFMLFLAGS} ${OPENGL_LIBRARIES})