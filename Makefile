# Компилятор и флаги
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude
LDFLAGS = -lssl -lcrypto

# Директории
SRC_DIR = src
INCLUDE_DIR = include
TESTS_DIR = tests
BUILD_DIR = build

# Исходные файлы
SERVER_SRCS = $(wildcard $(SRC_DIR)/*.cpp)
SERVER_OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SERVER_SRCS))

# Исключаем main.cpp для тестов и server_tests.cpp для сервера
SERVER_SRCS_NO_MAIN = $(filter-out $(SRC_DIR)/main.cpp,$(SERVER_SRCS))
SERVER_OBJS_NO_MAIN = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SERVER_SRCS_NO_MAIN))

TEST_SRCS = $(wildcard $(TESTS_DIR)/*.cpp)
TEST_OBJS = $(patsubst $(TESTS_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(TEST_SRCS))

# Целевые файлы
SERVER_TARGET = $(BUILD_DIR)/server
TEST_TARGET = $(BUILD_DIR)/server_tests

# Основная цель по умолчанию - компиляция сервера
all: $(SERVER_TARGET)

# Компиляция сервера (исключаем server_tests.cpp)
$(SERVER_TARGET): $(filter-out $(BUILD_DIR)/server_tests.o,$(SERVER_OBJS))
	@mkdir -p $(BUILD_DIR)
	$(CXX) $^ -o $@ $(LDFLAGS)

# Компиляция для тестов (исключаем main.cpp)
test: $(TEST_TARGET)

$(TEST_TARGET): $(SERVER_OBJS_NO_MAIN) $(TEST_OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $^ -o $@ $(LDFLAGS) -lUnitTest++

# Компиляция объектных файлов из src
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Компиляция объектных файлов из tests
$(BUILD_DIR)/%.o: $(TESTS_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Очистка - удаляем только build directory, tests остается нетронутым
clean:
	rm -rf $(BUILD_DIR)

# Пересборка
rebuild: clean all

# Пересборка тестов
retest: clean test

# Вспомогательные цели
.PHONY: all test clean rebuild retest

# Зависимости для заголовочных файлов
$(BUILD_DIR)/ClientSession.o: $(INCLUDE_DIR)/ClientSession.h
$(BUILD_DIR)/Config.o: $(INCLUDE_DIR)/Config.h
$(BUILD_DIR)/CryptoService.o: $(INCLUDE_DIR)/CryptoService.h
$(BUILD_DIR)/Logger.o: $(INCLUDE_DIR)/Logger.h
$(BUILD_DIR)/ProtocolHandler.o: $(INCLUDE_DIR)/ProtocolHandler.h
$(BUILD_DIR)/Server.o: $(INCLUDE_DIR)/Server.h
$(BUILD_DIR)/UserDatabase.o: $(INCLUDE_DIR)/UserDatabase.h
$(BUILD_DIR)/main.o: $(INCLUDE_DIR)/ClientSession.h $(INCLUDE_DIR)/Config.h $(INCLUDE_DIR)/CryptoService.h $(INCLUDE_DIR)/Logger.h $(INCLUDE_DIR)/ProtocolHandler.h $(INCLUDE_DIR)/Server.h $(INCLUDE_DIR)/UserDatabase.h
$(BUILD_DIR)/server_tests.o: $(INCLUDE_DIR)/ClientSession.h $(INCLUDE_DIR)/Config.h $(INCLUDE_DIR)/CryptoService.h $(INCLUDE_DIR)/Logger.h $(INCLUDE_DIR)/ProtocolHandler.h $(INCLUDE_DIR)/Server.h $(INCLUDE_DIR)/UserDatabase.h
