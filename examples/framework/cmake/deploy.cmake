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

	file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/mac_deploy.sh"
"#!/bin/bash
set -e
exe=\"\$1\"
dir=\"\$2\"
otool -L \"\$exe\" | grep ' /' | awk '{print \$1}' | while read dep; do
    fname=\$(basename \"\$dep\")
    dst=\"\$dir/\$fname\"
    echo \"copy file: \$dep -> \$dst\"
    cp -u \"\$dep\" \"\$dst\" 2>/dev/null || true
done
")

	file(CHMOD "${CMAKE_CURRENT_BINARY_DIR}/mac_deploy.sh" PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

	add_custom_command(TARGET ${target} POST_BUILD
		COMMAND bash "${CMAKE_CURRENT_BINARY_DIR}/mac_deploy.sh" "$<TARGET_FILE:${target}>" "$<TARGET_FILE_DIR:${target}>"
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

	# ldd de | grep "=> /" | awk '{print $3}' | while read dep; do fname=$(basename "$dep"); dst="./$fname"; echo "copy file: $dep -> $dst"; cp -u "$dep" "$dst"; done
	file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/linux_deploy.sh"
"#!/bin/bash
set -e
exe=\"$1\"
dir=\"$2\"
ldd \"\$exe\" | grep '=> /' | awk '{print \$3}' | while read dep; do
    fname=\$(basename \"\$dep\")
    dst=\"\$dir/\$fname\"
    echo \"copy file: \$dep -> \$dst\"
    cp -u \"\$dep\" \"\$dst\"
done
")

	file(CHMOD "${CMAKE_CURRENT_BINARY_DIR}/linux_deploy.sh" PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

	add_custom_command(TARGET ${target} POST_BUILD
		COMMAND bash "${CMAKE_CURRENT_BINARY_DIR}/linux_deploy.sh" "$<TARGET_FILE:${target}>" "$<TARGET_FILE_DIR:${target}>"
		COMMENT "Copying shared library dependencies (ldd)..."
	)
endfunction()