function(executable executable_name)
  set(multiValueArgs SOURCES LIBS)
  cmake_parse_arguments(executable "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
  add_executable(${executable_name} ${executable_SOURCES})
  target_link_libraries(${executable_name} ${executable_LIBS})
  if (HAVE_ASAN)
    target_compile_options(${executable_name} PUBLIC "-fsanitize=address")
    set_target_properties(${executable_name} PROPERTIES LINK_FLAGS "-fsanitize=address")
  endif()
endfunction(executable)

dump_variable(
Coin_LIBRARY
X11_LIBRARIES
X11_Xt_LIB
X11_Xpm_LIB
)

include_directories(
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/src>
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>)

if (NOT X11_Xpm_FOUND)
  message(WARNING "X11::Xpm missing some examples will not be built")
else()
  executable(button           SOURCES button.cpp LIBS SoXt)
endif()
executable(coloreditor      SOURCES coloreditor.cpp LIBS SoXt)
executable(examinerviewer   SOURCES examinerviewer.cpp LIBS SoXt)
executable(initalfullscreen SOURCES initalfullscreen.cpp LIBS SoXt)
executable(materialeditor   SOURCES materialeditor.cpp LIBS SoXt)
#executable(initialpos       SOURCES initialpos.c LIBS ${X11_Xt_LIB} ${X11_LIBRARIES})
