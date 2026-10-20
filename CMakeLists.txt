if (COMPONENTS_IS_ENABLED_QT)
    COMPONENTS_CONFIGURE_COMPONENT(Logger "1.0.0"
        Qt5::Core
    )
else()
    COMPONENTS_CONFIGURE_COMPONENT(Logger "1.0.0")
endif()

COMPONENTS_ADD_COMPONENT_TEST(Logger)

target_precompile_headers(Components-Logger PUBLIC
    "src/common.hpp"
    "src/filewriterbase.hpp"
    "src/instancebase.hpp"
)