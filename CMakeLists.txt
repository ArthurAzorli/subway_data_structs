cmake_minimum_required(VERSION 3.31)
project(Trabalho01 C)

set(CMAKE_C_STANDARD 11)

add_executable(Trabalho01
        project/lib/provided.h
        project/lib/provided.c
        project/services/file_repository.c
        project/services/file_repository.h
        project/services/split.h
        project/services/types.h
        project/subway/subway_header_repository.c
        project/subway/subway_header_repository.h
        project/subway/subway_record_repository.c
        project/subway/subway_record_repository.h
        project/subway/input/input_repository.c
        project/subway/input/input_repository.h
        project/subway/subway_record_list.c
        project/subway/subway_record_list.h
        project/subway/graph.c
        project/subway/graph.h
        project/indexable/indexable_record_avl.c
        project/indexable/indexable_record_avl.h
        project/indexable/indexable_record_repository.c
        project/indexable/indexable_record_repository.h
        project/services/search_service.c
        project/services/search_service.h
        project/services/types.c
        project/program.h
        project/program.c
        main.c
)
