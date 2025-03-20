find_path(ATOMIC_QUEUE_INCLUDE_DIRS "atomic_queue/atomic_queue.h")


add_executable(main
    main.cpp
)

target_include_directories(main PRIVATE ${ATOMIC_QUEUE_INCLUDE_DIRS})
