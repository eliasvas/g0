#include "profiler.h"

// FIXME: remove this, NO stdlib!!!
#include <stdio.h>

// HMMMMMM
u64 platform_read_cpu_timer();
u64 platform_read_cpu_freq();


// Whoops!
global_var Profiler g_prof;
global_var u32 g_parent_idx;


void profiler_begin() {
  g_prof.begin_cyc = platform_read_cpu_timer();
}

void profiler_end() {
  g_prof.end_cyc = platform_read_cpu_timer();
}

void profiler_print() {
  printf("---- PROFILER INFO ----\n");
  u64 total_cyc = g_prof.end_cyc - g_prof.begin_cyc;
  printf("Total duration: %lu cycles, (100%%)\n", total_cyc);

  for (u64 i = 0; i < MAX_PROFILE_REGIONS; ++i) {
    Profiler_Region *region = &g_prof.regions[i];
    if (region->elapsed_inclusive_cyc) {
    printf("Region %s(%u):", region->label, region->hit_count);
    printf("%lu cycles, (%.2f%%", region->elapsed_exclusive_cyc, 100.0 * ((f64)region->elapsed_exclusive_cyc / (f64)total_cyc));
    if (region->elapsed_inclusive_cyc != region->elapsed_exclusive_cyc) {
      f64 percent_elapsed_inclusive = 100.0 * ((f64)region->elapsed_inclusive_cyc / (f64)total_cyc);
      printf(", %.2f%% w/ children", percent_elapsed_inclusive);
    }
    printf(")\n");
    }
  }
  printf("-----------------------\n");
}

void profiler_end_and_print() {
  profiler_end();
  profiler_print();
}


Profiler_Block profiler_block_begin(const char *region_label, u64 region_idx) {
  Profiler_Region *region = &g_prof.regions[region_idx];

  Profiler_Block block = (Profiler_Block){
    .region_label = region_label,
    .region_idx = region_idx,
    .begin_cyc = platform_read_cpu_timer(),
    .old_elapsed_inclusive_cyc = region->elapsed_inclusive_cyc,
    .parent_idx = g_parent_idx, // By default its zero, which encodes no parent! we want NO invalid Region pointers OK!?!?!?
  };

  return block;
}
void profiler_block_end(Profiler_Block *block) {
  Profiler_Region *region = &g_prof.regions[block->region_idx];
  Profiler_Region *parent = &g_prof.regions[block->parent_idx];

  block->end_cyc = platform_read_cpu_timer();
  u64 delta = block->end_cyc - block->begin_cyc;

  region->hit_count += 1;
  region->elapsed_inclusive_cyc = block->old_elapsed_inclusive_cyc + delta;
  region->elapsed_exclusive_cyc += delta;
  parent->elapsed_exclusive_cyc -= delta;

  region->label = block->region_label;

  g_parent_idx = block->parent_idx; // reset the parent index to the blocks actual parent (It might have changed through execution)
}


