
function(deploy target)
	set(libs ${ARGN})
	if(WIN32)
		# Copy VC runtime libraries
		include(InstallRequiredSystemLibraries)
		foreach(lib ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})
			get_filename_component(filename "${lib}" NAME)
			add_custom_command(TARGET ${target} POST_BUILD
				COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${lib}" "$<TARGET_FILE_DIR:${target}>"
				COMMENT "Copying runtime: ${filename}..."
			)
		endforeach()
		# Copy dependency DLLs
		foreach(lib ${libs})
			get_filename_component(filename "${lib}" NAME)
			if(filename MATCHES ".*\\.dll$")
				add_custom_command(TARGET ${target} POST_BUILD
					COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${lib}" "$<TARGET_FILE_DIR:${target}>"
					COMMENT "Copying dep DLL: ${filename}..."
				)
			endif()
		endforeach()
	elseif(APPLE)
		# Copy dependency dylibs
		foreach(lib ${libs})
			get_filename_component(filename "${lib}" NAME)
			if(filename MATCHES ".*\\.dylib$")
				add_custom_command(TARGET ${target} POST_BUILD
					COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${lib}" "$<TARGET_FILE_DIR:${target}>"
					COMMENT "Copying dep dylib: ${filename}..."
				)
			endif()
		endforeach()
		# Use otool -L to analyze and copy all dependent dylibs
		add_custom_command(TARGET ${target} POST_BUILD
			COMMAND bash -c "otool -L $<TARGET_FILE:${target}> | grep ' /' | awk '{print $1}' | xargs -I '{}' cp -u '{}' $<TARGET_FILE_DIR:${target}> 2>/dev/null || true"
			COMMENT "Copying shared library dependencies (otool -L)..."
		)
	elseif(UNIX)
		# Copy dependency so
		foreach(lib ${libs})
			get_filename_component(filename "${lib}" NAME)
			if(filename MATCHES ".*\\.so.*$")
				add_custom_command(TARGET ${target} POST_BUILD
					COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${lib}" "$<TARGET_FILE_DIR:${target}>"
					COMMENT "Copying dep so: ${filename}..."
				)
			endif()
		endforeach()
		# Use ldd to analyze and copy all dependent so
		add_custom_command(TARGET ${target} POST_BUILD
			COMMAND bash -c "ldd $<TARGET_FILE:${target}> | grep '=> /' | awk '{print $3}' | xargs -I '{}' cp -u '{}' $<TARGET_FILE_DIR:${target}>"
			COMMENT "Copying shared library dependencies (ldd)..."
		)
	endif()
endfunction()
