if (TARGET Qt5::Core)
    COMPONENTS_CONFIGURE_COMPONENT(Logger
        Qt5::Core
    )
else()
    COMPONENTS_CONFIGURE_COMPONENT(Logger)
endif()
