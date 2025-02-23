#pragma once

#include <windows.h>

namespace Error
{
    // Manejo de excepciones
    class ExceptionManager
    {
        public:
            static LONG WINAPI manageMsvcException(PEXCEPTION_POINTERS exception);
            static void manageTerminate();
        
        private:
            static void manageMsvcException();
            static LONG manageCriticalMsvcException(PEXCEPTION_POINTERS exception);
    };
};
