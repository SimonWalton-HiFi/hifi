#
# QmlCompile.cmake
# cmake/macros
#
# Copyright (c) 2019 High Fidelity, Inc
#

macro(hifi_qml_compile)
  if (WIN32)
    find_program(QMLCACHEGEN_CMD qmlcachegen PATHS ${QT_DIR}/bin)

    if (NOT QMLCACHEGEN_CMD)
      message(WARNING "hifi_qml_compile: Couldn't find qmlcachegen, required to generate .qmlc")
    else()
			file(GLOB_RECURSE QML_FILES LIST_DIRECTORIES false
			  "$<TARGET_FILE_DIR:${TARGET_NAME}>/*.qml")

			foreach(QML_FILE ${QML_FILES})
			  add_custom_command(
					TARGET ${TARGET_NAME}
				  POST_BUILD
					COMMAND CMD /C "${QMLCACHEGEN_CMD} \"${QML_FILE}\""
				)
			endforeach(QML_FILE)
    endif()
	endif(WIN32)
    
endmacro(hifi_qml_compile)
