add_subdirectory(FunctionalSim)
add_subdirectory(ShISA)

install(
  FILES test.sh
  DESTINATION bin
  PERMISSIONS
    OWNER_READ
    OWNER_WRITE
    OWNER_EXECUTE
    GROUP_READ
    GROUP_WRITE
    GROUP_EXECUTE
    WORLD_READ
    WORLD_EXECUTE
  RENAME ShISA-test)
