#include "main/WorkMode.h"

namespace Main
{
    ////////////////////////////////////////////
    // Modo de trabajo del programa principal //
    ////////////////////////////////////////////

    ////////////////////////
    // Funciones de clase //
    ////////////////////////

    std::string WorkMode::getWorkModeDescription(Mode mode)
    {
        switch (mode)
        {
            case Mode::UNDEFINED:                    return "Sin definir";
            case Mode::THROW_CCP_EXCEPTION:          return "Lanzamiento de excepcion C++";
            case Mode::THROW_SEH_EXCEPTION:          return "Lanzamiento de excepcion SEH";
            case Mode::THROW_THREADED_CPP_EXCEPTION: return "Lanzamiento de excepcion C++ en un thread";
            case Mode::THROW_THREADED_SEH_EXCEPTION: return "Lanzamiento de excepcion SEH en un thread";
            default:                                 break;
        }

        return "N/A";
    }
};
