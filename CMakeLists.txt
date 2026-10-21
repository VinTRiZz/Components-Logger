COMPONENTS_CONFIGURE_COMPONENT(Logger "1.0.0")
target_precompile_headers(Components-Logger PUBLIC
    "src/common.hpp"
    "src/filewriterbase.hpp"
    "src/instancebase.hpp"
)
COMPONENTS_LINK_COMPONENT(Components-Logger Core)
if (COMPONENTS_IS_ENABLED_QT)
    target_link_libraries(Components-Logger PRIVATE Qt5::Core)
endif()
COMPONENTS_ADD_COMPONENT_TEST(Logger)
