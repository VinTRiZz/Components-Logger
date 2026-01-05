if (COMPONENTS_IS_ENABLED_QT)
    COMPONENTS_CONFIGURE_COMPONENT(Logger
        Qt5::Core
    )
target_compile_definitions(Logger PUBLIC COMPONENTS_LOGGER_USE_QT)
else()
    COMPONENTS_CONFIGURE_COMPONENT(Logger)
endif()
