include_directories(${FREETYPE2_INCLUDE_DIRS})

add_library(galaster
  galaster.cc
  geometry.cc
  glyph.cc
  font.cc
  bezier.cc
  vec3d.cc
  prop.cc
)

if (APPLE)
    find_library(COCOA_FRAMEWORK Cocoa)
    find_library(IOKIT_FRAMEWORK IOKit)
    find_library(CORE_FOUNDATION_FRAMEWORK CoreFoundation)
    find_library(CORE_VIDEO_FRAMEWORK CoreVideo)
    mark_as_advanced(COCOA_FRAMEWORK
                     IOKIT_FRAMEWORK
                     CORE_FOUNDATION_FRAMEWORK
                     CORE_VIDEO_FRAMEWORK)
    list(APPEND GALASTER_LIBRARIES "${COCOA_FRAMEWORK}"
                               "${IOKIT_FRAMEWORK}"
                               "${CORE_FOUNDATION_FRAMEWORK}"
                               "${CORE_VIDEO_FRAMEWORK}")
endif ()

if (APPLE)
target_link_libraries(galaster
  ${GALASTER_LIBRARIES}
  ${GLFW_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${FREETYPE2_LIBRARIES}
  /usr/local/lib/libiomp5.dylib
)
else ()
target_link_libraries(galaster
  ${GALASTER_LIBRARIES}
  ${GLFW_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${FREETYPE2_LIBRARIES}
)
endif ()
