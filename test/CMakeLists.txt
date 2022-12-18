
set(ADPLUG_TESTMUS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/testmus")

set(LIBRARIES
  adplug
  libbinio::libbinio
)

###
### crctest
###
add_executable(crctest crctest.cpp)
target_link_libraries(crctest PRIVATE ${LIBRARIES})

add_test(
  NAME crctest
  COMMAND crctest
  WORKING_DIRECTORY "${ADPLUG_TESTMUS_DIR}"
)

###
### emutest
###
add_executable(emutest emutest.cpp)
target_link_libraries(emutest PRIVATE ${LIBRARIES})

add_test(
  NAME emutest
  COMMAND emutest
  WORKING_DIRECTORY "${ADPLUG_TESTMUS_DIR}"
)

###
### playertest
###
add_executable(playertest playertest.cpp)
target_link_libraries(playertest PRIVATE ${LIBRARIES})

add_test(
  NAME playertest
  COMMAND playertest
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

###
### stresstest
###
if(NOT DEFINED WIN32)
  add_executable(stresstest stresstest.cpp)
  target_link_libraries(stresstest PRIVATE ${LIBRARIES})

  add_test(
    NAME stresstest
    COMMAND stresstest
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  )
endif()
