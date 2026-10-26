if(DESCENT_BUILD_CLIENT)
	find_package(Vulkan REQUIRED)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(WAYLAND REQUIRED IMPORTED_TARGET wayland-client)

	descent_library(rengine
		SOURCES rengine.cpp
		LINK
			Vulkan::Headers
			PkgConfig::WAYLAND
	)
endif()
