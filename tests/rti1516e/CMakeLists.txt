# The current standard fdd used for testing
SET(RTI1516E_FDD_FILE ${CMAKE_SOURCE_DIR}/share/rti1516e/HLAstandardMIM.xml)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include/rti1516e)
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/src/OpenRTI)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/OpenRTI)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)

IF(NOT OPENRTI_BUILD_SHARED)
  ADD_DEFINITIONS(-DSTATIC_RTI)
  ADD_DEFINITIONS(-DSTATIC_FEDTIME)
ENDIF()

# Test directories should be listed here in sensible testing order
ADD_SUBDIRECTORY(link)
ADD_SUBDIRECTORY(fddget)
ADD_SUBDIRECTORY(modules)
