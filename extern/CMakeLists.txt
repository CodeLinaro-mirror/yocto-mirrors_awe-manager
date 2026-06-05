

# The list of those libraries is manually updated in this folder
# They originate from a pre-built stage of a specific AWE-Q release
#
# They need to be updated whenever:
#  - the shmem layout is changed (this is hardcoded into awe_alsa lib)
#  - the packet client has some bug fixes
#  - general (Ease) abstractions have incompatible APIs


set(AWEQ_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/aweq/${AWEMGR_AWEQ_VERSION})


# AWE Alsa library - general carved shmem code

set(_libfile_alsa  "${AWEQ_DIR}/lib/libawe_alsa.so")
add_library(awe_alsa SHARED IMPORTED GLOBAL)
set_target_properties(awe_alsa PROPERTIES
    IMPORTED_LOCATION ${_libfile_alsa}
    INTERFACE_INCLUDE_DIRECTORIES "${AWEQ_DIR}/include"
)
install(FILES ${_libfile_alsa} TYPE LIB)

# Packet CLient - socket client code

set(_libfile_pclient  "${AWEQ_DIR}/lib/libawe_packetclient.so")
add_library(awe_packetclient STATIC IMPORTED GLOBAL)
set_target_properties(awe_packetclient PROPERTIES
    IMPORTED_LOCATION ${_libfile_pclient}
    INTERFACE_INCLUDE_DIRECTORIES "${AWEQ_DIR}/include"
    INTERFACE_LINK_LIBRARIES pthread
)
install(FILES ${_libfile_pclient} TYPE LIB)

# Some abstraction lib

set(_libfile_aliteosal  "${AWEQ_DIR}/lib/libawe_audiolite_osal.so")
add_library(awe_audiolite_osal STATIC IMPORTED GLOBAL)
set_target_properties(awe_audiolite_osal PROPERTIES
    IMPORTED_LOCATION ${_libfile_aliteosal}
    INTERFACE_INCLUDE_DIRECTORIES "${AWEQ_DIR}/include"
    INTERFACE_LINK_LIBRARIES ipcc
)
install(FILES ${_libfile_aliteosal} TYPE LIB)