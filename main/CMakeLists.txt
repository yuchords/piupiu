file(GLOB_RECURSE LIB_SRCS
    "../lib/pubsubclient/src/PubSubClient.cpp"


)

file(GLOB_RECURSE OUR_SRCS
    "network/net_manager.cc"
    "network/blufi_security.cpp"
        "network/blufi_basic.cpp"
)



idf_component_register(SRCS ${LIB_SRCS} ${OUR_SRCS} "main.cpp"
                    INCLUDE_DIRS "include" "../lib/pubsubclient/src"
                    "network"

)
