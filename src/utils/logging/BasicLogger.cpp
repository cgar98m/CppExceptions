#include "utils/logging/BasicLogger.h"

#include <iostream>
#include <string>
#include "utils/logging/LogTypes.h"

namespace Utils
{
    namespace Logging
    {
        //////////////////////////////////
        // Logger para salida estandar  //
        //////////////////////////////////

        //------------//
        // Constantes //
        //------------//
    
        const char  *BasicLogger::MUX_NAME           = "Utils/Logging/StdCoutMutex";
        const DWORD BasicLogger::TIMEOUT_MS_MUX_WAIT = 1000;
    
        //------------------------//
        // Constructor/Destructor //
        //------------------------//

        BasicLogger::~BasicLogger()
        {
            std::lock_guard<std::mutex> lock(this->internalMux);
            if (this->ostreamMux)
            {
                CloseHandle(this->ostreamMux);
                this->ostreamMux = nullptr;
            }
        }

        BasicLogger::BasicLogger()
            : ILogger()
            , ostreamMux(CreateMutex(nullptr, FALSE, MUX_NAME))
        {
        }

        //----------------//
        // Final virtual  //
        //----------------//

        bool BasicLogger::printRequest(const LogMsg &message)
        {
            return this->processPrint(message);
        }
    
        bool BasicLogger::processPrint(const LogMsg &message)
        {
            // Duplicamos el mutex
            HANDLE localPrintMutex = nullptr;
            {
                std::lock_guard<std::mutex> lock(this->internalMux);
                if (!this->ostreamMux) return false;
    
                HANDLE processHandle = GetCurrentProcess();
                if (!DuplicateHandle(processHandle
                    , this->ostreamMux
                    , processHandle
                    , &localPrintMutex
                    , 0
                    , FALSE
                    , DUPLICATE_SAME_ACCESS))
                {
                    return false;
                }
            }
            if (!localPrintMutex) return false;
    
            // Esperamos a tener la propiedad del mutex (con un tiempo maximo)
            if (WaitForSingleObject(localPrintMutex, TIMEOUT_MS_MUX_WAIT) != WAIT_OBJECT_0) return false;
    
            // Limitamos el tamano del texto
            std::string shortenedText(message.text, 0, LOGGER_PRINT_LIMIT_SIZE);
    
            // Printamos el mensaje
            for (size_t idx = 0; idx < shortenedText.size(); idx += LOGGER_PRINT_BUFFER_SIZE)
            {
                std::cout << std::string(shortenedText, idx, LOGGER_PRINT_BUFFER_SIZE);
            }
            std::cout << std::endl;
            
            // Liberamos la propiedad del mutex y cerramos el handle
            ReleaseMutex(localPrintMutex);
            CloseHandle(localPrintMutex);
            return true;
        }

        //--------------------//
        // Funciones de clase //
        //--------------------//

        SharedLogger BasicLogger::getInstance()
        {
            std::lock_guard<std::mutex> lock(instanceMux);
            if (!basicLogger) basicLogger.reset(new BasicLogger());
            return basicLogger;
        }

        //--------------------//
        // Variables de clase //
        //--------------------//
        
        SharedLogger BasicLogger::basicLogger;
        std::mutex   BasicLogger::instanceMux;
    };
};
