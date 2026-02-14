file(GLOB_RECURSE LIB_SRCS
    "../lib/pubsubclient/src/PubSubClient.cpp"


)

file(GLOB_RECURSE OUR_SRCS
    "apps/*.cpp"
    # "apps/app_base/*.cpp"
    "audio/*.cpp"
)


idf_component_register(SRCS ${LIB_SRCS} ${OUR_SRCS} "main.cpp"
                    INCLUDE_DIRS "../lib/pubsubclient/src"
                    "apps"
                    "audio"
)
