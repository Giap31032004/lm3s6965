#include "kernel.h"



const char* process_state_str(process_state_t state) {
    switch (state) {
        case PROC_UNUSED:         return "UNUSED";
        case PROC_NEW:            return "NEW";
        case PROC_READY:          return "READY";
        case PROC_RUNNING:        return "RUNNING";
        case PROC_WAITING_TIME:   return "WAIT_TIME";   // Rõ ràng hơn BLOCKED
        case PROC_WAITING_OBJECT: return "WAIT_OBJ";    // Rõ ràng hơn BLOCKED
        case PROC_WAITING_IO:     return "WAIT_IO";
        case PROC_SUSPENDED:      return "SUSPENDED";
        case PROC_TERMINATED:     return "DEAD";
        default:                  return "UNKNOWN";
    }
}