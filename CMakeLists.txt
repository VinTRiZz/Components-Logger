if (TARGET Qt5::Core)
    COMPONENTS_CONFIGURE_COMPONENT(Logger
        Qt5::Core
    )

target_compile_definitions(Logger PUBLIC LOGGER_USE_QT)
else()
    COMPONENTS_CONFIGURE_COMPONENT(Logger)
endif()
