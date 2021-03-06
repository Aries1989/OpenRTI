# The directory containing the public api for rti1516e
SET(RTI1516E_HEADER_PATH ${CMAKE_SOURCE_DIR}/include/rti1516e)
SET(RTI1516E_DATA_PATH ${CMAKE_SOURCE_DIR}/share/rti1516e)

# All the -I arguments
INCLUDE_DIRECTORIES(${RTI1516E_HEADER_PATH})
# The OpenRTI implementation files and may be build generated sources
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/src/OpenRTI)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/OpenRTI)

INCLUDE(CreateResource)
CREATE_RESOURCE(${CMAKE_SOURCE_DIR}/share/rti1516e/HLAstandardMIM.xml
  HLAstandardMIM_xml ${CMAKE_CURRENT_BINARY_DIR}/HLAstandardMIM.inc)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

SET(RTI1516E_PUBLIC_HEADERS
        ${RTI1516E_HEADER_PATH}/RTI/Enums.h
        ${RTI1516E_HEADER_PATH}/RTI/Exception.h
        ${RTI1516E_HEADER_PATH}/RTI/FederateAmbassador.h
        ${RTI1516E_HEADER_PATH}/RTI/Handle.h
        ${RTI1516E_HEADER_PATH}/RTI/LogicalTimeFactory.h
        ${RTI1516E_HEADER_PATH}/RTI/LogicalTime.h
        ${RTI1516E_HEADER_PATH}/RTI/LogicalTimeInterval.h
        ${RTI1516E_HEADER_PATH}/RTI/RangeBounds.h
        ${RTI1516E_HEADER_PATH}/RTI/NullFederateAmbassador.h
        ${RTI1516E_HEADER_PATH}/RTI/RTI1516.h
        ${RTI1516E_HEADER_PATH}/RTI/RTIambassadorFactory.h
        ${RTI1516E_HEADER_PATH}/RTI/RTIambassador.h
        ${RTI1516E_HEADER_PATH}/RTI/SpecificConfig.h
        ${RTI1516E_HEADER_PATH}/RTI/Typedefs.h
        ${RTI1516E_HEADER_PATH}/RTI/VariableLengthData.h
)

SET(RTI1516E_PUBLIC_ENCODING_HEADERS
        ${RTI1516E_HEADER_PATH}/RTI/encoding/BasicDataElements.h
        ${RTI1516E_HEADER_PATH}/RTI/encoding/DataElement.h
        ${RTI1516E_HEADER_PATH}/RTI/encoding/EncodingExceptions.h
        ${RTI1516E_HEADER_PATH}/RTI/encoding/EncodingConfig.h
        ${RTI1516E_HEADER_PATH}/RTI/encoding/HLAfixedArray.h
        ${RTI1516E_HEADER_PATH}/RTI/encoding/HLAfixedRecord.h
        ${RTI1516E_HEADER_PATH}/RTI/encoding/HLAopaqueData.h
        ${RTI1516E_HEADER_PATH}/RTI/encoding/HLAvariableArray.h
        ${RTI1516E_HEADER_PATH}/RTI/encoding/HLAvariantRecord.h
)

SET(RTI1516E_PUBLIC_TIME_HEADERS
        ${RTI1516E_HEADER_PATH}/RTI/time/HLAfloat64Interval.h
        ${RTI1516E_HEADER_PATH}/RTI/time/HLAfloat64TimeFactory.h
        ${RTI1516E_HEADER_PATH}/RTI/time/HLAfloat64Time.h
        ${RTI1516E_HEADER_PATH}/RTI/time/HLAinteger64Interval.h
        ${RTI1516E_HEADER_PATH}/RTI/time/HLAinteger64TimeFactory.h
        ${RTI1516E_HEADER_PATH}/RTI/time/HLAinteger64Time.h
)

SET(LIB_RTI1516E_SOURCES
        BasicDataElements.cpp
        DataElement.cpp
        EncodingExceptions.cpp
        Exception.cpp
        FederateAmbassador.cpp
        Handle.cpp
        HLAfixedArray.cpp
        HLAfixedRecord.cpp
        HLAopaqueData.cpp
        HLAvariableArray.cpp
        HLAvariantRecord.cpp
        HLAfloat64Interval.cpp
        HLAfloat64Time.cpp
        HLAfloat64TimeFactory.cpp
        HLAinteger64Interval.cpp
        HLAinteger64Time.cpp
        HLAinteger64TimeFactory.cpp
        HLAlogicalTimeFactoryFactory.cpp
        LogicalTime.cpp
        LogicalTimeFactory.cpp
        LogicalTimeInterval.cpp
        NullFederateAmbassador.cpp
        RangeBounds.cpp
        VariableLengthData.cpp
        Version.cpp
        RTI1516ELogicalTimeFactory.cpp
        RTIambassador.cpp
        RTIambassadorFactory.cpp
        RTIambassadorImplementation.cpp
        Typedefs.cpp
)

SET(LIB_FEDTIME1516E_SOURCES
        LogicalTimeFactoryFactory.cpp
)

IF(OPENRTI_BUILD_SHARED)
  ADD_LIBRARY(rti1516e SHARED ${LIB_RTI1516E_SOURCES})
  SET_PROPERTY(TARGET rti1516e APPEND PROPERTY COMPILE_DEFINITIONS BUILDING_RTI)
  SET_PROPERTY(TARGET rti1516e PROPERTY VERSION 1.0.0)
  SET_PROPERTY(TARGET rti1516e PROPERTY SOVERSION 1)

  # rti1516e, the circular reference is tricky on win32
  IF(MSVC)
    ADD_LIBRARY(fedtime1516eStub STATIC ${LIB_FEDTIME1516E_SOURCES})
    SET_PROPERTY(TARGET fedtime1516eStub APPEND PROPERTY COMPILE_DEFINITIONS BUILDING_FEDTIME)
    # Ideally we could use
    #  SET_PROPERTY(TARGET fedtime1516eStub APPEND PROPERTY STATIC_LIBRARY_FLAGS_DEBUG "/DEF /NAME:$<TARGET_FILE:fedtime1516e>")
    # to get the final target file name as it is produced, but STATIC_LIBRARY_FLAGS does not evaluate generator expressions
    # So use different variants of CMAKE_<CONFIG>_POSTFIX
    OPENRTI_SET_MSVC_STUB_DLL_NAME(fedtime1516eStub libfedtime1516e)
    OPENRTI_TARGET_LINK_PRIVATE_LIBRARIES(rti1516e fedtime1516eStub OpenRTI)
  ELSEIF(MINGW OR CYGWIN)
    ADD_LIBRARY(fedtime1516eStub STATIC ${LIB_FEDTIME1516E_SOURCES})
    SET_PROPERTY(TARGET fedtime1516eStub APPEND PROPERTY COMPILE_DEFINITIONS BUILDING_FEDTIME)
    STRING(TOUPPER ${CMAKE_BUILD_TYPE} _CONFIG)
    SET(_fedtimeDllName libfedtime1516e${CMAKE_${_CONFIG}_POSTFIX}.dll)
    ADD_CUSTOM_COMMAND(TARGET fedtime1516eStub
      POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E remove -f ${_fedtimeDllName}.a
      COMMAND "${DLLTOOL}" -D${_fedtimeDllName} -l${_fedtimeDllName}.a $<TARGET_FILE:fedtime1516eStub>
      COMMAND "${CMAKE_COMMAND}" -E remove -f $<TARGET_FILE:fedtime1516eStub>
      COMMAND "${CMAKE_COMMAND}" -E rename ${_fedtimeDllName}.a $<TARGET_FILE:fedtime1516eStub>
      )
    OPENRTI_TARGET_LINK_PRIVATE_LIBRARIES(rti1516e fedtime1516eStub OpenRTI)
  ELSE()
    OPENRTI_TARGET_LINK_PRIVATE_LIBRARIES(rti1516e OpenRTI)
    IF(OPENRTI_INSTALL_WITH_RPATH)
      SET_PROPERTY(TARGET rti1516e PROPERTY INSTALL_RPATH "$ORIGIN")
      SET_PROPERTY(TARGET rti1516e APPEND PROPERTY LINK_FLAGS "${OPENRTI_RPATH_LINK_FLAGS}")
    ENDIF()
  ENDIF()
  IF(MACOS)
    SET_PROPERTY(TARGET rti1516e APPEND PROPERTY LINK_FLAGS "-Wl,-undefined,dynamic_lookup ${OPENRTI_BIND_AT_LOAD}")
  ENDIF()
ELSE()
  ADD_LIBRARY(rti1516e STATIC ${LIB_RTI1516E_SOURCES})
  SET_PROPERTY(TARGET rti1516e APPEND PROPERTY COMPILE_DEFINITIONS STATIC_RTI STATIC_FEDTIME)
  SET_PROPERTY(TARGET rti1516e PROPERTY LINK_INTERFACE_MULTIPLICITY 2)
  OPENRTI_TARGET_POSITION_INDEPENDENT(rti1516e)
  OPENRTI_TARGET_LINK_PRIVATE_LIBRARIES(rti1516e fedtime1516e OpenRTI)
ENDIF()
IF(MSVC)
  SET_PROPERTY(TARGET rti1516e PROPERTY PREFIX "lib")
ENDIF()

# libfedtime1516e
IF(OPENRTI_BUILD_SHARED)
  ADD_LIBRARY(fedtime1516e SHARED ${LIB_FEDTIME1516E_SOURCES})
  SET_PROPERTY(TARGET fedtime1516e APPEND PROPERTY COMPILE_DEFINITIONS BUILDING_FEDTIME)
  SET_PROPERTY(TARGET fedtime1516e PROPERTY VERSION 1.0.0)
  SET_PROPERTY(TARGET fedtime1516e PROPERTY SOVERSION 1)
  IF(OPENRTI_INSTALL_WITH_RPATH)
    SET_PROPERTY(TARGET fedtime1516e PROPERTY INSTALL_RPATH "$ORIGIN")
    SET_PROPERTY(TARGET fedtime1516e APPEND PROPERTY LINK_FLAGS "${OPENRTI_RPATH_LINK_FLAGS}")
  ENDIF()
ELSE()
  ADD_LIBRARY(fedtime1516e STATIC ${LIB_FEDTIME1516E_SOURCES})
  SET_PROPERTY(TARGET fedtime1516e APPEND PROPERTY COMPILE_DEFINITIONS STATIC_RTI STATIC_FEDTIME)
  SET_PROPERTY(TARGET fedtime1516e PROPERTY LINK_INTERFACE_MULTIPLICITY 2)
  OPENRTI_TARGET_POSITION_INDEPENDENT(fedtime1516e)
ENDIF()
IF(MSVC)
  SET_PROPERTY(TARGET fedtime1516e PROPERTY PREFIX "lib")
ENDIF()
OPENRTI_TARGET_LINK_PRIVATE_LIBRARIES(fedtime1516e rti1516e OpenRTI)

INSTALL(TARGETS fedtime1516e rti1516e
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")
INSTALL(FILES ${RTI1516E_PUBLIC_HEADERS}
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${OPENRTI_RTI1516E_INCLUDE_SUBDIR}/RTI")
INSTALL(FILES ${RTI1516E_PUBLIC_ENCODING_HEADERS}
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${OPENRTI_RTI1516E_INCLUDE_SUBDIR}/RTI/encoding")
INSTALL(FILES ${RTI1516E_PUBLIC_TIME_HEADERS}
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${OPENRTI_RTI1516E_INCLUDE_SUBDIR}/RTI/time")
INSTALL(FILES ${RTI1516E_DATA_PATH}/HLAstandardMIM.xml
  DESTINATION "${OPENRTI_DATAROOTDIR}/rti1516e")

IF(UNIX)
  INCLUDE(PkgConfig)
  CREATE_PKG_CONFIG(rti1516e "HLA/RTI 1516E Communication Library." "${OPENRTI_RTI1516E_INCLUDE_SUBDIR}" "")
  CREATE_PKG_CONFIG(fedtime1516e "HLA/RTI 1516E Time Library." "${OPENRTI_RTI1516E_INCLUDE_SUBDIR}" "")
  CREATE_API_PKG_CONFIG(hla-rti1516e "HLA/RTI 1516E Libraries." rti1516e fedtime1516e)
ENDIF()
