/* ************************************************************************** *
 * Chess UCI                                                                  *
 * Universal Chess Interface for Chess Engines                                *
 * ************************************************************************** */

#ifndef CHESSUCI_PROCESS_FACTORY_H
#define CHESSUCI_PROCESS_FACTORY_H

#include "chessuci/engine_process.h"

#include <memory>

namespace chessuci {

class ProcessFactory {
public:
    static auto create(const ProcessParams &params) -> std::unique_ptr<EngineProcess>;
};

} // namespace chessuci

#endif
