/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#ifndef CHESSUCI_ENGINE_PROCESS_UNIX_H
#define CHESSUCI_ENGINE_PROCESS_UNIX_H

#include "chessuci/engine_process.h"

namespace chessuci {

class EngineProcessUnix : public EngineProcess {
public:
    EngineProcessUnix(const ProcessParams &params);
    ~EngineProcessUnix() override = default;
};

} // namespace chessuci

#endif
