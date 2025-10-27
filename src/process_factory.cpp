/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#include "chessuci/process_factory.h"

#if defined(CHESSUCI_WINDOWS)
#include "chessuci/engine_process_win.h"
using LocalEngineProcess = chessuci::EngineProcessWin;
#elif defined(CHESSUCI_UNIX)
#include "chessuci/engine_process_unix.h"
using LocalEngineProcess = chessuci::EngineProcessUnix;
#else
#error "Platform not configured in CMake"
#endif

namespace chessuci {

auto ProcessFactory::create_local() -> std::unique_ptr<EngineProcess> {
    return std::make_unique<LocalEngineProcess>();
}

} // namespace chessuci
