# Компилятор и флаги
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
LDFLAGS = -lssl -lcrypto

# Цели
TARGET_SERVER = server
TARGET_CLIENT = client_uint64_t
SOURCE_SERVER = servak_1.cpp

# Основная цель
all: $(TARGET_SERVER)

# Компиляция сервера
$(TARGET_SERVER): $(SOURCE_SERVER)
	$(CXX) $(CXXFLAGS) -o $(TARGET_SERVER) $(SOURCE_SERVER) $(LDFLAGS)

# Очистка
clean:
	rm -f $(TARGET_SERVER) *.o core

# Перекомпиляция
rebuild: clean all

# Запуск сервера (пример)
run: $(TARGET_SERVER)
	./$(TARGET_SERVER) users.db server.log 33333

# Отладочная сборка
debug: CXXFLAGS += -g -DDEBUG
debug: rebuild

# Установка зависимостей (для Ubuntu/Debian)
install-deps:
	sudo apt update
	sudo apt install g++ libssl-dev

# Помощь
help:
	@echo "Доступные цели:"
	@echo "  all      - компиляция сервера (по умолчанию)"
	@echo "  clean    - удаление скомпилированных файлов"
	@echo "  rebuild  - перекомпиляция"
	@echo "  run      - запуск сервера на порту 33333"
	@echo "  debug    - отладочная сборка"
	@echo "  install-deps - установка зависимостей"
	@echo "  help     - эта справка"

.PHONY: all clean rebuild run debug install-deps help
