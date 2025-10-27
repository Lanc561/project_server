#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <random>
#include <openssl/evp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctime>
#include <algorithm>

std::ofstream logfile;
std::unordered_map<std::string, std::string> user_db;

void log_message(const std::string& msg, const std::string& severity) {
    time_t now = time(0);
    struct tm* timeinfo = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
    
    logfile << "[" << timestamp << "] [" << severity << "] " << msg << std::endl;
    logfile.flush();
    
    std::cout << "[" << timestamp << "] [" << severity << "] " << msg << std::endl;
}

bool load_user_database(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        log_message("Не удалось открыть файл базы пользователей: " + filename, "CRITICAL");
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string login = line.substr(0, pos);
            std::string password = line.substr(pos + 1);
            user_db[login] = password;
        }
    }
    file.close();
    
    log_message("База пользователей загружена, пользователей: " + std::to_string(user_db.size()), "INFO");
    return true;
}

std::string md5(const std::string& str) {
    EVP_MD_CTX* context = EVP_MD_CTX_new();
    if (!context) {
        throw std::runtime_error("Failed to create MD5 context");
    }
    
    if (EVP_DigestInit_ex(context, EVP_md5(), nullptr) != 1) {
        EVP_MD_CTX_free(context);
        throw std::runtime_error("Failed to initialize MD5 digest");
    }
    
    if (EVP_DigestUpdate(context, str.c_str(), str.length()) != 1) {
        EVP_MD_CTX_free(context);
        throw std::runtime_error("Failed to update MD5 digest");
    }
    
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len;
    if (EVP_DigestFinal_ex(context, digest, &digest_len) != 1) {
        EVP_MD_CTX_free(context);
        throw std::runtime_error("Failed to finalize MD5 digest");
    }
    
    EVP_MD_CTX_free(context);
    
    std::stringstream ss;
    for (unsigned int i = 0; i < digest_len; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);
    }
    return ss.str();
}

std::string to_upper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

uint64_t calculate_sum(const std::vector<uint64_t>& vec) {
    if(vec.empty()) return 0;
    
    uint64_t sum = 0;
    for(auto val : vec) {
        if(val > UINT64_MAX - sum) {
            return UINT64_MAX;
        }
        sum += val;
    }
    return sum;
}

bool recv_all(int sock, void* buf, size_t len) {
    char* p = static_cast<char*>(buf);
    while(len > 0) {
        ssize_t received = recv(sock, p, len, 0);
        if(received <= 0) {
            return false;
        }
        p += received;
        len -= received;
    }
    return true;
}

bool send_all(int sock, const void* buf, size_t len) {
    const char* p = static_cast<const char*>(buf);
    while(len > 0) {
        ssize_t sent = send(sock, p, len, 0);
        if(sent <= 0) {
            return false;
        }
        p += sent;
        len -= sent;
    }
    return true;
}

void handle_client(int client_sock) {
    char buffer[256];
    
    // Аутентификация
    int len = recv(client_sock, buffer, 52, 0);
    if(len != 52) {
        log_message("Ошибка получения auth данных", "WARNING");
        close(client_sock);
        return;
    }
    
    std::string auth_data(buffer, 52);
    std::string login = auth_data.substr(0, 4);
    std::string salt = auth_data.substr(4, 16);
    std::string client_hash = auth_data.substr(20, 32);
    
    log_message("Получено auth сообщение: " + auth_data, "INFO");
    log_message("Разобранное сообщение - Логин: " + login + ", Соль: " + salt + ", Хеш клиента: " + client_hash, "INFO");
    
    if(user_db.find(login) == user_db.end()) {
        log_message("Пользователь не найден: " + login, "WARNING");
        send(client_sock, "ERR", 3, 0);
        close(client_sock);
        return;
    }
    
    std::string password = user_db[login];
    log_message("Пароль из базы: " + password, "INFO");
    
    std::string dataToHash = salt + password;
    std::string expectedHash = md5(dataToHash);
    log_message("Данные для хеширования: " + dataToHash, "INFO");
    log_message("Ожидаемый хеш: " + expectedHash, "INFO");
    
    std::string clientHashUpper = to_upper(client_hash);
    std::string expectedHashUpper = to_upper(expectedHash);
    log_message("Сравнение - Хеш клиента: " + clientHashUpper + ", Ожидаемый: " + expectedHashUpper, "INFO");
    
    if(clientHashUpper != expectedHashUpper) {
        log_message("Неверный пароль для пользователя: " + login, "WARNING");
        send(client_sock, "ERR", 3, 0);
        close(client_sock);
        return;
    }
    
    send(client_sock, "OK", 2, 0);
    log_message("Успешная аутентификация: " + login, "INFO");
    log_message("Отправлен ответ OK клиенту", "INFO");
    
    // Получение информации о клиенте для логирования
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    getpeername(client_sock, (struct sockaddr*)&client_addr, &addr_len);
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    
    log_message("Аутентификация пройдена для " + std::string(client_ip) + ", начинаем обработку данных", "INFO");
    log_message("Начало обработки данных клиента", "INFO");
    
    // Обработка векторов
    uint32_t num_vectors;
    if(!recv_all(client_sock, &num_vectors, sizeof(num_vectors))) {
        log_message("Ошибка чтения количества векторов", "WARNING");
        close(client_sock);
        return;
    }
    
    log_message("Количество векторов для обработки: " + std::to_string(num_vectors), "INFO");
    
    for(uint32_t i = 0; i < num_vectors; i++) {
        log_message("Обработка вектора " + std::to_string(i + 1), "INFO");
        
        uint32_t vec_size;
        if(!recv_all(client_sock, &vec_size, sizeof(vec_size))) {
            log_message("Ошибка чтения размера вектора", "WARNING");
            close(client_sock);
            return;
        }
        
        log_message("Размер вектора " + std::to_string(i + 1) + ": " + std::to_string(vec_size), "INFO");
        
        std::vector<uint64_t> vector(vec_size);
        if(vec_size > 0) {
            if(!recv_all(client_sock, vector.data(), vec_size * sizeof(uint64_t))) {
                log_message("Ошибка чтения данных вектора", "WARNING");
                close(client_sock);
                return;
            }
            
            log_message("Успешно прочитан вектор " + std::to_string(i + 1) + ", байт: " + 
                       std::to_string(vec_size * sizeof(uint64_t)), "INFO");
            
            // Логируем содержимое вектора только для отладки и если вектор небольшой
            if (vec_size <= 10) {
                std::string vectorContents = "Содержимое вектора " + std::to_string(i + 1) + ": ";
                for (uint32_t j = 0; j < vector.size(); ++j) {
                    vectorContents += std::to_string(vector[j]);
                    if (j < vector.size() - 1) vectorContents += ", ";
                }
                log_message(vectorContents, "DEBUG");
            }
        }
        
        uint64_t result = calculate_sum(vector);
        log_message("Вектор " + std::to_string(i + 1) + " обработан. Результат: " + std::to_string(result), "INFO");
        
        if(!send_all(client_sock, &result, sizeof(result))) {
            log_message("Ошибка отправки результата", "WARNING");
            close(client_sock);
            return;
        }
    }
    
    log_message("Обработка завершена для " + std::string(client_ip), "INFO");
    close(client_sock);
}

int main(int argc, char* argv[]) {
    if(argc == 2 && std::string(argv[1]) == "-h") {
        std::cout << "Использование: " << argv[0] << " <файл_базы> <файл_журнала> <порт>" << std::endl;
        return 0;
    }
    
    if(argc != 4) {
        std::cerr << "Неверные аргументы. Используйте -h для справки" << std::endl;
        return 1;
    }
    
    logfile.open(argv[2], std::ios::app);
    if(!logfile.is_open()) {
        std::cerr << "Не удалось открыть файл журнала" << std::endl;
        return 1;
    }
    
    if (!load_user_database(argv[1])) {
        std::cerr << "Ошибка загрузки базы пользователей" << std::endl;
        return 1;
    }
    
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(server_sock < 0) {
        log_message("Ошибка создания сокета", "CRITICAL");
        return 1;
    }
    
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(std::stoi(argv[3]));
    addr.sin_addr.s_addr = INADDR_ANY;
    
    if(bind(server_sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        log_message("Ошибка привязки сокета к порту " + std::string(argv[3]), "CRITICAL");
        close(server_sock);
        return 1;
    }
    
    if(listen(server_sock, 5) < 0) {
        log_message("Ошибка начала прослушивания", "CRITICAL");
        close(server_sock);
        return 1;
    }
    
    log_message("Сервер инициализирован на порту " + std::string(argv[3]), "INFO");
    log_message("Сервер запущен", "INFO");
    
    while(true) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
        if(client_sock < 0) {
            log_message("Ошибка принятия соединения", "WARNING");
            continue;
        }
        
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        log_message("Новое подключение от " + std::string(client_ip), "INFO");
        
        handle_client(client_sock);
    }
    
    close(server_sock);
    return 0;
}
