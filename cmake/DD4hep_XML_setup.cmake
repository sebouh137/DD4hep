if(DD4HEP_USE_XERCESC)
  find_package( XercesC REQUIRED )
  set_target_properties(XercesC::XercesC
    PROPERTIES
    COMPILE_DEFINITIONS DD4HEP_USE_XERCESC
    INTERFACE_COMPILE_DEFINITIONS DD4HEP_USE_XERCESC
    )
  set(XML_LIBRARIES XercesC::XercesC)
else()
  set(DD4HEP_USE_XERCESC OFF)
  ADD_LIBRARY(TinyXML INTERFACE)
  TARGET_COMPILE_DEFINITIONS(TinyXML INTERFACE DD4HEP_USE_TINYXML)
  set(XML_LIBRARIES TinyXML)
  INSTALL(TARGETS TinyXML EXPORT DD4hep DESTINATION lib)
endif()
