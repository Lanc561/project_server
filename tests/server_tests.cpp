#include <UnitTest++/UnitTest++.h>
#include "Config.h"
#include "Server.h"
#include "UserDatabase.h"
#include "CryptoService.h"
#include "Logger.h"
#include "ProtocolHandler.h"
#include "ClientSession.h"
#include <fstream>
#include <filesystem>
#include <sstream>
#include <iostream>
using namespace std;

// Вспомогательный класс для перехвата вывода
class OutputRedirect {
private:
    streambuf* oldCout;
    streambuf* oldCerr;
    ostringstream coutBuffer;
    ostringstream cerrBuffer;

public:
    OutputRedirect() {
        oldCout = cout.rdbuf(coutBuffer.rdbuf());
        oldCerr = cerr.rdbuf(cerrBuffer.rdbuf());
    }
    
    ~OutputRedirect() {
        cout.rdbuf(oldCout);
        cerr.rdbuf(oldCerr);
    }
};

// Тесты для UserDatabase
SUITE(UserDatabaseTest) {
    TEST(LoadValidDatabase) {
        ofstream testFile("test_users.db");
        testFile << "user1:password123\nuser2:qwerty\n";
        testFile.close();
        
        UserDatabase db("test_users.db");
        CHECK(db.load());
        
        filesystem::remove("test_users.db");
    }
    
    TEST(LoadInvalidDatabase) {
        UserDatabase db("nonexistent.db");
        CHECK(!db.load());
    }
    
    TEST(ValidUserCheck) {
        ofstream testFile("test_users.db");
        testFile << "user1:password123\nuser2:qwerty\n";
        testFile.close();
        
        UserDatabase db("test_users.db");
        db.load();
        CHECK(db.isValidUser("user1"));
        
        filesystem::remove("test_users.db");
    }
    
    TEST(InvalidUserCheck) {
        ofstream testFile("test_users.db");
        testFile << "user1:password123\n";
        testFile.close();
        
        UserDatabase db("test_users.db");
        db.load();
        CHECK(!db.isValidUser("nonexistent"));
        
        filesystem::remove("test_users.db");
    }
    
    TEST(VerifyCorrectPassword) {
        ofstream testFile("test_users.db");
        testFile << "user1:password123\n";
        testFile.close();
        
        UserDatabase db("test_users.db");
        db.load();
        CHECK(db.verifyPassword("user1", "password123"));
        
        filesystem::remove("test_users.db");
    }
    
    TEST(VerifyIncorrectPassword) {
        ofstream testFile("test_users.db");
        testFile << "user1:password123\n";
        testFile.close();
        
        UserDatabase db("test_users.db");
        db.load();
        CHECK(!db.verifyPassword("user1", "wrongpassword"));
        
        filesystem::remove("test_users.db");
    }
    
    TEST(GetPasswordForValidUser) {
        ofstream testFile("test_users.db");
        testFile << "user1:password123\n";
        testFile.close();
        
        UserDatabase db("test_users.db");
        db.load();
        CHECK_EQUAL("password123", db.getPassword("user1"));
        
        filesystem::remove("test_users.db");
    }
    
    TEST(GetPasswordForInvalidUser) {
        ofstream testFile("test_users.db");
        testFile << "user1:password123\n";
        testFile.close();
        
        UserDatabase db("test_users.db");
        db.load();
        CHECK_EQUAL("", db.getPassword("nonexistent"));
        
        filesystem::remove("test_users.db");
    }
}

// Тесты для CryptoService
SUITE(CryptoServiceTest) {
    TEST(ComputeMD5) {
        string result = CryptoService::computeMD5("test");
        CHECK_EQUAL("098f6bcd4621d373cade4e832627b4f6", result);
    }
    
    TEST(ComputeMD5EmptyString) {
        string result = CryptoService::computeMD5("");
        CHECK_EQUAL("d41d8cd98f00b204e9800998ecf8427e", result);
    }
    
    TEST(ComputeMD5LongString) {
        string result = CryptoService::computeMD5("hello world");
        CHECK_EQUAL("5eb63bbbe01eeed093cb22bb8f5acdc3", result);
    }
    
    TEST(GenerateSaltUnique) {
        uint64_t salt1 = CryptoService::generateSalt();
        uint64_t salt2 = CryptoService::generateSalt();
        CHECK(salt1 != salt2);
    }
    
    TEST(SaltToHexString) {
        uint64_t salt = 0x1234567890ABCDEF;
        string result = CryptoService::saltToHexString(salt);
        CHECK_EQUAL("1234567890abcdef", result);
    }
    
    TEST(SaltToHexStringZero) {
        uint64_t salt = 0;
        string result = CryptoService::saltToHexString(salt);
        CHECK_EQUAL("0000000000000000", result);
    }
}

// Тесты для Logger
SUITE(LoggerTest) {
    TEST(CreateLogger) {
        Logger logger("test.log");
        CHECK(true);
        filesystem::remove("test.log");
    }
    
    TEST(LogToFile) {
        string filename = "test_log.log";
        filesystem::remove(filename);
        
        Logger logger(filename);
        logger.log(LogLevel::INFO, "Test info message");
        logger.log(LogLevel::WARNING, "Test warning message");
        logger.log(LogLevel::ERROR, "Test error message");
        
        CHECK(filesystem::exists(filename));
        filesystem::remove(filename);
    }
}

// Тесты для Config
SUITE(ConfigTest) {
    TEST(ParseValidCommandLine) {
        OutputRedirect redirect; // Перехватываем вывод
        
        Config config;
        char* argv[] = {
            (char*)"server",
            (char*)"-d", (char*)"users.db",
            (char*)"-l", (char*)"server.log", 
            (char*)"-p", (char*)"8080"
        };
        
        CHECK(config.parseCommandLine(7, argv));
        CHECK_EQUAL("users.db", config.getUserDbFile());
        CHECK_EQUAL("server.log", config.getLogFile());
        CHECK_EQUAL(8080, config.getPort());
    }
    
    TEST(ParseInvalidCommandLineMissingParams) {
        OutputRedirect redirect; // Перехватываем вывод
        
        Config config;
        char* argv[] = {
            (char*)"server",
            (char*)"-d", (char*)"users.db"
        };
        
        CHECK(!config.parseCommandLine(3, argv));
    }
    
    TEST(ParseHelpShort) {
        OutputRedirect redirect; // Перехватываем вывод
        
        Config config;
        char* argv[] = {
            (char*)"server",
            (char*)"-h"
        };
        
        CHECK(!config.parseCommandLine(2, argv));
    }
    
    TEST(ParseHelpLong) {
        OutputRedirect redirect; // Перехватываем вывод
        
        Config config;
        char* argv[] = {
            (char*)"server",
            (char*)"--help"
        };
        
        CHECK(!config.parseCommandLine(2, argv));
    }
}

// Тесты для ClientSession
SUITE(ClientSessionTest) {
    TEST(CreateSession) {
        UserDatabase db("test.db");
        Logger logger("test.log");
        ClientSession session(0, db, logger);
        CHECK(!session.isAuthenticated());
        
        filesystem::remove("test.db");
        filesystem::remove("test.log");
    }
}

// Тесты для граничных случаев
SUITE(EdgeCasesTest) {
    TEST(UserDatabaseEmptyFile) {
        ofstream testFile("empty.db");
        testFile.close();
        
        UserDatabase db("empty.db");
        CHECK(db.load());
        
        filesystem::remove("empty.db");
    }
    
    TEST(UserDatabaseMalformedFile) {
        ofstream testFile("malformed.db");
        testFile << "user1password123\n";
        testFile << ":password\n";
        testFile << "user2:\n";
        testFile.close();
        
        UserDatabase db("malformed.db");
        CHECK(db.load());
        
        filesystem::remove("malformed.db");
    }
    
    TEST(CryptoServiceMD5SpecialCharacters) {
        string result = CryptoService::computeMD5("hello@world#123");
        CHECK(!result.empty());
        CHECK_EQUAL(32, result.length());
    }
}

// Главная функция тестирования
int main() {
    // Создаем тестовые файлы перед запуском всех тестов
    ofstream userDb("test_users.db");
    userDb << "user1:password123\nuser2:qwerty\nadmin:admin123\n";
    userDb.close();

    // Запускаем все тесты
    int result = UnitTest::RunAllTests();

    // Удаляем тестовые файлы после выполнения
    filesystem::remove("test_users.db");
    filesystem::remove("test.log");
    filesystem::remove("test_log.log");
    filesystem::remove("empty.db");
    filesystem::remove("malformed.db");
    filesystem::remove("test.db");

    return result;
}
