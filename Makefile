BUILD:=build
CLIENT_BUILD_DIR:= $(BUILD)/client
SERVER_BUILD_DIR:= $(BUILD)/server

CLIENT_BUILD_TARGET:= $(CLIENT_BUILD_DIR)/client
SERVER_BUILD_TARGET:= $(SERVER_BUILD_DIR)/server

CLIENT_CXX_FILES:= $(wildcard AsyncClient/*.cpp)
SERVER_CXX_FILES:= $(wildcard AsyncServer/*.cpp)

CLIENT_OBJ_FILES:= $(CLIENT_CXX_FILES:client/%.cpp=$(CLIENT_BUILD_DIR)/%_client_cpp.o)
SERVER_OBJ_FILES:= $(SERVER_CXX_FILES:server/%.cpp=$(SERVER_BUILD_DIR)/%_server_cpp.o)

CXX:= clang++
CXXFLAGS:= -std=c++20 -g -Wall -I/usr/local/include -L/usr/local/lib -specs=libjsoncpp.dylib libprotobuf.dylib


LDFLAGS:= 

.PHONY: clean all
all:$(SERVER_BUILD_TARGET) $(CLIENT_BUILD_TARGET)

$(CLIENT_BUILD_TARGET):$(CLIENT_CXX_FILES)
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $^ -o $@
	cp lib/*.dylib $(CLIENT_BUILD_DIR)

$(SERVER_BUILD_TARGET):$(SERVER_CXX_FILES)
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $^ -o $@
	cp lib/*.dylib $(SERVER_BUILD_DIR)

clean:
	rm -rf $(BUILD)
