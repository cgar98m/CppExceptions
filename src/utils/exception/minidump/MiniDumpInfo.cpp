#include "utils/exception/minidump/MiniDumpInfo.h"

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

        bool MiniDumpInfo::isValid() const
        {
            return process && processId && threadId;
        }
    };
};
