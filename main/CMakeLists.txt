file(GLOB_RECURSE LIB_SRCS
    "../lib/pubsubclient/src/PubSubClient.cpp"


)


idf_component_register(SRCS ${LIB_SRCS} "main.cpp" "my_wifi.cpp" "my_mqtt.cpp" "my_parse.cpp" "my_dht11.cpp"
                    INCLUDE_DIRS "include" "../lib/pubsubclient/src"

)
