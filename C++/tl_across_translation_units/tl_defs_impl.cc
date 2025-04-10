#include "tl_defs.hh"
#include <cstddef>

thread_local uint8_t big_tl[TL_SIZE_BYTES] = {1};

void dummy_cross_translation_units_unused() {}
