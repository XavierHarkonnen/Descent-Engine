add_subdirectory(server)
add_subdirectory(bsl)

if(DESCENT_BUILD_CLIENT)
	add_subdirectory(client)
endif()
