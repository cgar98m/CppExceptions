#include "utils/exception/RequiredExceptionInfo.h"

namespace Utils
{
    namespace Exception
    {
        //////////////////////////////////////////////////
        // Informacion minima para generar un mini dump //
        //////////////////////////////////////////////////
    
        //--------------------//
        // Funciones miembro  //
        //--------------------//

        bool RequiredExceptionInfo::isValid() const
        {
            return process && processId && threadId && exception;
        }

        bool RequiredExceptionInfo::isFullyValid() const
        {
            return process && processId && thread && threadId && exception;
        }
    };
};
