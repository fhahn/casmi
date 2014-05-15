#include "libinterpreter/symbolic.h"

namespace symbolic {
  static uint32_t last_id = 0;

  uint32_t next_id() {
    last_id += 1;
    return last_id;
  }
}
