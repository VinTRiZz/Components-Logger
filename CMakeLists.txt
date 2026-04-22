if (COMPONENTS_IS_ENABLED_QT)
    COMPONENTS_CONFIGURE_COMPONENT(Logger
        Qt5::Core
    )
else()
    COMPONENTS_CONFIGURE_COMPONENT(Logger)
endif()

COMPONENTS_ADD_COMPONENT_TEST(Logger)

target_precompile_headers(Logger PUBLIC
    "src/common.hpp"
    "src/filewriterbase.hpp"
    "src/instancebase.hpp"
)