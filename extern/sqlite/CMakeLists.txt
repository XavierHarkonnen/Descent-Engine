include(FetchContent)

FetchContent_Declare(
	sqlite
	GIT_REPOSITORY https://github.com/sqlite/sqlite.git
	GIT_TAG version-3.53.3
)

FetchContent_MakeAvailable(sqlite)