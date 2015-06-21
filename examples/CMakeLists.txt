include_directories(${CMAKE_SOURCE_DIR}/src)

macro(add_example name ...)
  add_executable(${name} ${ARGV})
  target_link_libraries(${name} galaster)
endmacro(add_example)

add_example(binary_tree binary_tree.cc)
add_example(cube cube.cc)
add_example(membrane membrane.cc)
add_example(multiedge multiedge.cc)
add_example(random random.cc)
add_example(randspline randspline.cc)
add_example(splineorama2 splineorama2.cc)
add_example(splineorama splineorama.cc)
add_example(label label.cc)
add_example(crawler crawler.cc)

