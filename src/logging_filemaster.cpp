#include "logging_filemaster.hpp"

namespace Logging
{

LoggingFileMaster::LoggingFileMaster(const std::string &filePath) :
#ifdef QT_CORE_LIB
    logfile(filePath.c_str())
#else
    logfilePath{ filePath }
#endif // QT_CORE_LIB
{

}

LoggingFileMaster::~LoggingFileMaster() {

}

}
