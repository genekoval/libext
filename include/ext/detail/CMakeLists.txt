target_sources(ext PUBLIC FILE_SET HEADERS FILES
    bit.hpp
    dynarray.hpp
    pool.hpp
    scope.hpp
)

add_subdirectory(coroutine)
