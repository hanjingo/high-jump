if(${CMAKE_VERSION} VERSION_LESS "3.22.1")
	function(win_deploy target)
		set(libs ${ARGN})
		include(InstallRequiredSystemLibraries)
		foreach(lib ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})
			get_filename_component(filename "${lib}" NAME)
			set(dst "$<TARGET_FILE_DIR:${target}>/${filename}")
			add_custom_command(TARGET ${target} POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E echo "copy file: ${lib} -> ${dst}"
				COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${lib}" "${dst}"
				COMMENT "Copying runtime: ${filename}..."
			)
		endforeach()
		foreach(lib ${libs})
			get_filename_component(filename "${lib}" NAME)
			if(filename MATCHES ".*\\.dll$")
				set(dst "$<TARGET_FILE_DIR:${target}>/${filename}")
				add_custom_command(TARGET ${target} POST_BUILD
					COMMAND ${CMAKE_COMMAND} -E echo "copy file: ${lib} -> ${dst}"
					COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${lib}" "${dst}"
					COMMENT "Copying dep DLL: ${filename}..."
				)
			endif()
		endforeach()
	endfunction()

	function(mac_deploy target)
		set(libs ${ARGN})
		foreach(lib ${libs})
			get_filename_component(filename "${lib}" NAME)
			if(filename MATCHES ".*\\.dylib$")
				set(dst "$<TARGET_FILE_DIR:${target}>/${filename}")
				add_custom_command(TARGET ${target} POST_BUILD
					COMMAND ${CMAKE_COMMAND} -E echo "copy file: ${lib} -> ${dst}"
					COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${lib}" "${dst}"
					COMMENT "Copying dep dylib: ${filename}..."
				)
			endif()
		endforeach()
		add_custom_command(TARGET ${target} POST_BUILD
			COMMAND bash -c 'otool -L $<TARGET_FILE:${target}> | grep " /" | awk "{print \$1}" | while read dep; do fname=$(basename "$dep"); dst="$<TARGET_FILE_DIR:${target}>/$fname"; echo "copy file: $dep -> $dst"; cp -u "$dep" "$dst" 2>/dev/null || true; done'
			COMMENT "Copying shared library dependencies (otool -L)..."
		)
	endfunction()

	function(linux_deploy target)
		set(libs ${ARGN})
		foreach(lib ${libs})
			get_filename_component(filename "${lib}" NAME)
			if(filename MATCHES ".*\\.so.*$")
				set(dst "$<TARGET_FILE_DIR:${target}>/${filename}")
				add_custom_command(TARGET ${target} POST_BUILD
					COMMAND ${CMAKE_COMMAND} -E echo "copy file: ${lib} -> ${dst}"
					COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${lib}" "${dst}"
					COMMENT "Copying dep so: ${filename}..."
				)
			endif()
		endforeach()
		add_custom_command(TARGET ${target} POST_BUILD
			COMMAND bash -c 'ldd $<TARGET_FILE:${target}> | grep "=> /" | awk "{print \$3}" | while read dep; do fname=$(basename "$dep"); dst="$<TARGET_FILE_DIR:${target}>/$fname"; echo "copy file: $dep -> $dst"; cp -u "$dep" "$dst"; done'
			COMMENT "Copying shared library dependencies (ldd)..."
		)
	endfunction()
else()
	function(_deploy target)
		get_target_property(target_path ${target} RUNTIME_OUTPUT_DIRECTORY)
		if(NOT target_path)
			set(target_path "${CMAKE_CURRENT_BINARY_DIR}/../bin")
		endif()
		set(exe "${target_path}/${target}")

		set(script "${CMAKE_CURRENT_BINARY_DIR}/copy_runtime_${target}.cmake")
		file(WRITE "${script}" "
	message(STATUS \"Analyzing and copying runtime dependencies for ${exe}\")
	file(GET_RUNTIME_DEPENDENCIES
		EXECUTABLES \"${exe}\"
		RESOLVED_DEPENDENCIES_VAR resolved
		UNRESOLVED_DEPENDENCIES_VAR unresolved
	)
	foreach(dep IN LISTS resolved)
		if(EXISTS \"\${dep}\")
			get_filename_component(fname \"\${dep}\" NAME)
			file(COPY \"\${dep}\" DESTINATION \"${target_path}\")
			message(STATUS \"Copied: \${dep}\")
		endif()
	endforeach()
	")

		add_custom_command(TARGET ${target} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -P "${script}"
			COMMENT "Copying shared library dependencies..."
		)
	endfunction()

	function(win_deploy target)
		_deploy(${target} ${ARGN})
	endfunction()

	function(mac_deploy target)
		_deploy(${target} ${ARGN})
	endfunction()

	function(linux_deploy target)
		_deploy(${target} ${ARGN})
	endfunction()

endif()