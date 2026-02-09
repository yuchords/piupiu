file(GLOB_RECURSE LIB_SRCS
    "../lib/pubsubclient/src/PubSubClient.cpp"


)





idf_component_register(SRCS ${LIB_SRCS} "main.cpp"
                    INCLUDE_DIRS "include" "../lib/pubsubclient/src"

)
