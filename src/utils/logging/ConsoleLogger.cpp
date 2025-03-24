#include "utils/logging/ConsoleLogger.h"

#include <iostream>
#include "utils/logging/LogEntry.h"
#include "utils/logging/LogTypes.h"

namespace Utils
{
    namespace Logging
    {
        ////////////////////////////////////////////////////////
        // Logger para salida estandar con impresion por hilo //
        ////////////////////////////////////////////////////////
    
        //------------//
        // Constantes //
        //------------//

        const char  *ConsoleLogger::MUX_NAME           = BasicLogger::MUX_NAME;
        const DWORD ConsoleLogger::TIMEOUT_MS_MUX_WAIT = BasicLogger::TIMEOUT_MS_MUX_WAIT;
        const char  *ConsoleLogger::LOGGER_NAME        = "ConsoleLogger";
    
        //------------------------//
        // Constructor/Destructor //
        //------------------------//
    
        ConsoleLogger::ConsoleLogger(const SharedLogger &logger)
            : IThreadedLogger(Params{THREAD_PRIORITY_NORMAL, TIMEOUT_MS_LOOP_WAIT, TIMEOUT_MS_STOP_WAIT, TIMEOUT_MS_MUX_WAIT, LOGGER_NAME}
                            , logger)
        {
            // Usamos el mismo nombre de mutex que el logger de salida estandar
            this->ostreamMux = CreateMutex(nullptr, FALSE, MUX_NAME);
        }
        
        //----------------//
        // Final virtual  //
        //----------------//

        bool ConsoleLogger::validateStream(const LogMsg & message)
        {
            return true;
        }

        bool ConsoleLogger::processPrintToStream(const std::string &message)
        {
            // Printamos el mensaje
            for (size_t idx = 0; idx < message.size(); idx += LOGGER_PRINT_BUFFER_SIZE)
            {
                std::cout << std::string(message, idx, LOGGER_PRINT_BUFFER_SIZE);
            }
            std::cout << std::endl;
            
            return true;
        }
    
        //--------------------//
        // Funciones de clase //
        //--------------------//

        SharedLogger ConsoleLogger::getInstance(const SharedLogger &logger)
        {
            std::lock_guard<std::mutex> lock(instanceMux);
            if (!consoleLogger) consoleLogger.reset(new ConsoleLogger(logger));
            return consoleLogger;
        }

        //--------------------//
        // Variables de clase //
        //--------------------//

        SharedLogger ConsoleLogger::consoleLogger;
        std::mutex   ConsoleLogger::instanceMux;
    };
};
