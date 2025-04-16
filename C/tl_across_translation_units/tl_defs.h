#pragma once

// A relatively big thread local area size, to dominate memory usage and
// improve check accuracy.
#define TL_SIZE_BYTES (2 * 1024 * 1024)

#define PAGE_SIZE (4096) // Assuming a 4KiB regular page size
static_assert(TL_SIZE_BYTES >= 2 * 1024 * 1024, "Thread local area size is too small for the check to be reliable");
static_assert(TL_SIZE_BYTES % PAGE_SIZE == 0, "Unaligned thread local area size");

void dummy_cross_translation_units_unused();
