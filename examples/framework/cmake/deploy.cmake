function(win_deploy target)
	set(libs ${ARGN})
	# Copy VC runtime libraries
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

	# Copy dependency DLLs
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
	# Copy dependency dylibs
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

	# Use otool -L to analyze and copy all dependent dylibs
	add_custom_command(TARGET ${target} POST_BUILD
		COMMAND bash -c 'otool -L $<TARGET_FILE:${target}> | grep " /" | awk "{print \$1}" | while read dep; do fname=$(basename "$dep"); dst="$<TARGET_FILE_DIR:${target}>/$fname"; echo "copy file: $dep -> $dst"; cp -u "$dep" "$dst" 2>/dev/null || true; done'
		COMMENT "Copying shared library dependencies (otool -L)..."
	)
endfunction()


function(linux_deploy target)
	# Copy dependency so
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

	# Use ldd to analyze and copy all dependent so
	add_custom_command(TARGET ${target} POST_BUILD
		COMMAND bash -c 'ldd $<TARGET_FILE:${target}> | grep "=> /" | awk "{print \$3}" | while read dep; do fname=$(basename "$dep"); dst="$<TARGET_FILE_DIR:${target}>/$fname"; echo "copy file: $dep -> $dst"; cp -u "$dep" "$dst"; done'
		COMMENT "Copying shared library dependencies (ldd)..."
	)
endfunction()