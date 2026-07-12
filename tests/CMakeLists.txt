file(GLOB test-sources CONFIGURE_DEPENDS *.cpp)

add_custom_target(notre-test)

foreach (source IN LISTS test-sources)
  get_filename_component(test "${source}" NAME_WE)
  add_library(notre-test-${test} STATIC EXCLUDE_FROM_ALL ${source})
  target_link_libraries(notre-test-${test} PRIVATE notre)
  add_dependencies(notre-test notre-test-${test})
  
  target_compile_options(notre-test-${test} PUBLIC "$<$<C_COMPILER_ID:MSVC>:/utf-8>")
  target_compile_options(notre-test-${test} PUBLIC "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endforeach()


