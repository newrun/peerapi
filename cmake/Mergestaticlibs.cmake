macro (MERGE_STATIC_LIBRARIES TARGET_LIB LIBRARIES LIBRARIES_DEBUG OBJECTS OBJECTS_DEBUG)

  if("${CMAKE_CFG_INTDIR}" STREQUAL ".")
    set(multiconfig FALSE)
  else()
    set(multiconfig TRUE)
  endif()

	if (WIN32)

		# On Windows you must add aditional formatting to the LIBRARIES variable as a single string for the windows libtool
		# with each library path wrapped in "" in case it contains spaces

		string (REPLACE ";" "\" \"" LIBS "${LIBRARIES}")
		string (REPLACE ";" "\" \"" LIBS_DEBUG "${LIBRARIES_DEBUG}")
		set (LIBS \"${LIBS}\")
		set (LIBS_DEBUG \"${LIBS_DEBUG}\")

    string (REPLACE ";" "\" \"" OBJS "${OBJECTS}")
		string (REPLACE ";" "\" \"" OBJS_DEBUG "${OBJECTS_DEBUG}")
		set (OBJS \"${OBJS}\")
		set (OBJS_DEBUG \"${OBJS_DEBUG}\")

    foreach(CONFIG_TYPE ${CMAKE_CONFIGURATION_TYPES})
      string(TOUPPER "${CONFIG_TYPE}" _CONFIG_TYPE)
      string(TOUPPER "STATIC_LIBRARY_FLAGS_${CONFIG_TYPE}" PROPNAME)
      if ("${_CONFIG_TYPE}" STREQUAL "DEBUG")
  			set_property (TARGET ${TARGET_LIB} APPEND PROPERTY ${PROPNAME} "${LIBS_DEBUG} ${OBJS_DEBUG}")
      else()
	  		set_property (TARGET ${TARGET_LIB} APPEND PROPERTY ${PROPNAME} "${LIBS} ${OBJS}")
      endif()
    endforeach()

  elseif (APPLE)

    # Use OSX's libtool to merge archives
    if(multiconfig)
      message(FATAL_ERROR "Multiple configurations are not supported")
    endif()

    set(outfile $<TARGET_FILE:${TARGET_LIB}>)  
    set(target_temp_file "${CMAKE_CURRENT_BINARY_DIR}/lib${TARGET_LIB}_temp.a")
  
    string(TOUPPER "${CMAKE_BUILD_TYPE}" _CMAKE_BUILD_TYPE)
    if (_CMAKE_BUILD_TYPE STREQUAL "DEBUG")
      set(_LIBRARIES ${LIBRARIES_DEBUG})
      set(_OBJECTS ${OBJECTS_DEBUG})
    else()
      set(_LIBRARIES ${LIBRARIES})
      set(_OBJECTS ${OBJECTS})
    endif()

    add_custom_command(TARGET ${TARGET_LIB} POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy "${outfile}"
                                "${target_temp_file}"
                        COMMAND rm "${outfile}"
                        COMMAND /usr/bin/libtool -no_warning_for_no_symbols -static -o "${outfile}"
                                "${target_temp_file}" ${_LIBRARIES} ${_OBJECTS}
                        COMMAND rm "${target_temp_file}"
                        )
  elseif (UNIX)

    # general UNIX - need to "ar -M"
    if(multiconfig)
      message(FATAL_ERROR "Multiple configurations are not supported")
    endif()

    string(TOUPPER "${CMAKE_BUILD_TYPE}" _CMAKE_BUILD_TYPE)
    if (_CMAKE_BUILD_TYPE STREQUAL "DEBUG")
      set(_LIBRARIES ${LIBRARIES_DEBUG})
      set(_OBJECTS ${OBJECTS_DEBUG})
    else()
      set(_LIBRARIES ${LIBRARIES})
      set(_OBJECTS ${OBJECTS})
    endif()

    set(outfile "libpeerconnect.a")
    set(target_temp_file "${CMAKE_CURRENT_BINARY_DIR}/lib${TARGET_LIB}_temp.a")

    set(MRI_SCRIPT "create ${outfile}\n")
    foreach(lib ${_LIBRARIES})
      set(MRI_SCRIPT "${MRI_SCRIPT}addlib ${lib}\n")
    endforeach()
    foreach(obj ${_OBJECTS})
      set(MRI_SCRIPT "${MRI_SCRIPT}addmod ${obj}\n")
    endforeach()
    set(MRI_SCRIPT "${MRI_SCRIPT}addlib ${target_temp_file}\n")
    set(MRI_SCRIPT "${MRI_SCRIPT}delete messagehandler.o\n")
    set(MRI_SCRIPT "${MRI_SCRIPT}save\n")
    set(MRI_SCRIPT "${MRI_SCRIPT}end\n")

    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/peerconnect.mri" "${MRI_SCRIPT}")

    add_custom_command(TARGET ${TARGET_LIB} POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy "${outfile}"
                                "${target_temp_file}"
                        COMMAND rm "${outfile}"
                        COMMAND ${CMAKE_AR} -M < "${CMAKE_CURRENT_BINARY_DIR}/peerconnect.mri"
                        COMMAND rm "${target_temp_file}"
                        )
	endif (WIN32)
endmacro (MERGE_STATIC_LIBRARIES)

