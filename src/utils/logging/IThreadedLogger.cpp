#include "utils/logging/IThreadedLogger.h"

#include "utils/logging/LogEntry.h"
#include "utils/logging/LogTypes.h"

namespace Utils
{
    namespace Logging
    {
        ////////////////////////////////////////
        // Hilo con gestion de logs por cola  //
        ////////////////////////////////////////

        //------------//
        // Constantes //
        //------------//
    
        const DWORD IThreadedLogger::TIMEOUT_MS_STOP_WAIT = 10000;
        const DWORD IThreadedLogger::TIMEOUT_MS_LOOP_WAIT = 0;
        const DWORD IThreadedLogger::TIMEOUT_MS_MUX_WAIT  = BasicLogger::TIMEOUT_MS_MUX_WAIT;
        const char  *IThreadedLogger::LOGGER_NAME         = "IThreadedLogger";
    
        //------------------------//
        // Constructor/Destructor //
        //------------------------//
    
        IThreadedLogger::IThreadedLogger(const Params &params, const SharedLogger &logger)
            : Thread::IQueueThread<LogMsg>(params.threadName, TIMEOUT_MS_DATA_WAIT, logger)
            , ILogger()
            , LoggerHolder(logger)
            , muxWait(params.muxWait)
            , printThread(*this
                        , Thread::ThreadHolder::Params{params.threadPriority, params.loopWait, params.stopWait}
                        , logger)
        {
        }

        IThreadedLogger::~IThreadedLogger()
        {
            std::lock_guard<std::mutex> lockPrint(this->internalMux);
            if (this->ostreamMux)
            {
                CloseHandle(this->ostreamMux);
                this->ostreamMux = nullptr;
            }
        }
    
        //----------------//
        // Final virtual  //
        //----------------//
        
        bool IThreadedLogger::printRequest(const LogMsg &message)
        {
            if (!this->printThread.run()) return false;
            return this->pushData(message);
        }
    
        bool IThreadedLogger::processData(const LogMsg &data)
        {
            return this->processPrint(data);
        }
    
        bool IThreadedLogger::processPrint(const LogMsg &message)
        {
            if (message.text.empty()) return true;

            // Duplicamos el mutex
            HANDLE localPrintMutex = nullptr;
            {
                std::lock_guard<std::mutex> lock(this->internalMux);
                if (!this->ostreamMux)
                {
                    LOGGER_THIS_LOG_ERROR() << "Mutex NO inicializado";
                    return false;
                }
    
                HANDLE processHandle = GetCurrentProcess();
                if (!DuplicateHandle(processHandle
                    , this->ostreamMux
                    , processHandle
                    , &localPrintMutex
                    , 0
                    , FALSE
                    , DUPLICATE_SAME_ACCESS))
                {
                    LOGGER_THIS_LOG_ERROR() << "Mutex NO duplicado: " << GetLastError();
                    return false;
                }
            }
            if (!localPrintMutex)
            {
                LOGGER_THIS_LOG_ERROR() << "Mutex NO valido";
                return false;
            }
    
            // Esperamos a tener la propiedad del mutex (con un tiempo maximo)
            DWORD waitResult = WaitForSingleObject(localPrintMutex, this->muxWait);
            if (waitResult != WAIT_OBJECT_0)
            {
                if (waitResult != WAIT_TIMEOUT) LOGGER_THIS_LOG_WARNING() << "TIMEOUT esperando mutex";
                else                            LOGGER_THIS_LOG_ERROR() << "ERROR esperando mutex: " << GetLastError();
                return false;
            }
    
            // Validamos el stream en base al mensaje
            if (!this->validateStream(message)) return false;

            // Limitamos el tamano del texto
            std::string shortenedText(message.text, 0, LOGGER_PRINT_LIMIT_SIZE);

            // Realizamos la impresion en el stream correspondiente
            bool printResult = this->processPrintToStream(shortenedText);
            
            // Liberamos la propiedad del mutex y cerramos el handle
            ReleaseMutex(localPrintMutex);
            CloseHandle(localPrintMutex);
            return printResult;
        }
    };
};
