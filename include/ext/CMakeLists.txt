target_sources(ext PUBLIC FILE_SET HEADERS FILES
    algorithm.h
    async_pool
    bit
    chrono.h
    coroutine
    data_size.h
    dynarray
    except.h
    json.hpp
    math.h
    pool
    scope
    string.h
    unix.h
)

add_subdirectory(detail)
