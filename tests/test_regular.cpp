#include <gtest/gtest.h>

#include <Components/Logger/Logger.h>

#include <fstream>
#include <filesystem>
#include <regex>

TEST(LoggerComponent, SetupDirectory) {
    const std::string testDirpath {"test"};
    if (std::filesystem::exists(testDirpath)) {
        std::filesystem::remove_all(testDirpath);
    }
    auto direcreateRes = std::filesystem::create_directory(testDirpath);
    ASSERT_TRUE(direcreateRes);
    COMPLOG_SET_LOGSDIR(testDirpath);

    const std::string testString {"TestString"};
    COMPLOG_SYNC_DEBUG  (testString);
    COMPLOG_SYNC_INFO   (testString);
    COMPLOG_SYNC_OK     (testString);
    COMPLOG_SYNC_WARNING(testString);
    COMPLOG_SYNC_ERROR  (testString);

    ASSERT_TRUE(std::filesystem::exists(COMPLOG_GET_LOGFILE())) << "Logging file did not created. File path:" << COMPLOG_GET_LOGFILE();

    // Check if written
    std::ifstream logfileReader(COMPLOG_GET_LOGFILE().data());
    ASSERT_TRUE(logfileReader.is_open()) << "Failed to open log file";
    std::string iStr;
    iStr.resize(2048);
    auto readSize = logfileReader.readsome(iStr.data(), iStr.size());
    iStr.resize(readSize);

    std::regex rexp(testString);
    auto writtenBeg = std::sregex_iterator(iStr.begin(), iStr.end(), rexp);
    auto writtenEnd = std::sregex_iterator();
    auto writtenCount = std::distance(writtenBeg, writtenEnd);
    ASSERT_EQ(writtenCount, 5) << "Invalid count of data strings written into file";

    std::filesystem::remove_all(testDirpath);
}
