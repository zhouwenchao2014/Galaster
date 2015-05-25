add_library(galaster
  galaster.cc
  geometry.cc
  bezier.cc
  vec3d.cc
  prop.cc
)

target_link_libraries(galaster
  ${GLFW_LIBRARIES}
  ${OPENGL_LIBRARIES}
)
