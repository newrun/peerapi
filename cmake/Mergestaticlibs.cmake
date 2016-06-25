macro (MERGE_STATIC_LIBRARIES TARGET LIBRARIES LIBRARIES_DEBUG)

  if("${CMAKE_CFG_INTDIR}" STREQUAL ".")
    set(multiconfig FALSE)
  else()
    set(multiconfig TRUE)
  endif()

  if ("${LIBRARIES_DEBUG}" STREQUAL "")
    set(LIBRARIES_DEBUG ${LIBRARIES})
  endif()

	if (WIN32)

		# On Windows you must add aditional formatting to the LIBRARIES variable as a single string for the windows libtool
		# with each library path wrapped in "" in case it contains spaces

		string (REPLACE ";" "\" \"" LIBS "${LIBRARIES}")
		string (REPLACE ";" "\" \"" LIBS_DEBUG "${LIBRARIES_DEBUG}")
		set (LIBS \"${LIBS}\")
		set (LIBS_DEBUG \"${LIBS_DEBUG}\")

    foreach(CONFIG_TYPE ${CMAKE_CONFIGURATION_TYPES})
      string(TOUPPER "${CONFIG_TYPE}" _CONFIG_TYPE)
      string(TOUPPER "STATIC_LIBRARY_FLAGS_${CONFIG_TYPE}" PROPNAME)
      if ("${_CONFIG_TYPE}" STREQUAL "DEBUG")
  			set_property (TARGET ${TARGET} APPEND PROPERTY ${PROPNAME} "${LIBS_DEBUG}")
      else()
	  		set_property (TARGET ${TARGET} APPEND PROPERTY ${PROPNAME} "${LIBS}")
      endif()
    endforeach()

	endif (WIN32)
endmacro (MERGE_STATIC_LIBRARIES)

