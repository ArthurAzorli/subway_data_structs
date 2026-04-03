cmake_minimum_required(VERSION 3.31)
project(Trabalho01 C)

set(CMAKE_C_STANDARD 11)

add_executable(Trabalho01
        project/core/utils/types.h
        project/core/utils/string.h
        project/core/utils/errors.h
        project/core/file/file_repository.c
        project/core/file/file_repository.h
        project/domain/subway_record.h
        project/domain/subway_record.c
        project/domain/session.h
        project/domain/session.c
        project/service/input/input_repository.h
        project/service/input/input_repository.c
        project/service/database/data_base_repository.c
        project/service/database/data_base_repository.h
        project/service/database/record_repository.c
        project/service/database/record_repository.h
        project/service/database/header_repository.c
        project/service/database/header_repository.h
        project/lib/provided.h
        project/lib/provided.c
        project/program.h
        project/program.c
        main.c)
