# install PerApp.yaml to /etc/urm/custom
install(
  DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/urm/custom/
  FILES_MATCHING
  PATTERN "*.yaml"
  PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)

# Install the extension "RestunePlugin.so" lib
add_library(RestunePlugin ${CMAKE_CURRENT_SOURCE_DIR}/Utils/Setup.cpp)
set_target_properties(RestunePlugin PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_link_libraries(RestunePlugin UrmExtAPIs)
# install to standard /usr/lib or /lib
install(TARGETS RestunePlugin DESTINATION ${CMAKE_INSTALL_LIBDIR})
