#pragma once

#define BONSAI_HASH_TABLE 1
// 0 = BonsaiTable -> tdc::compact_hash::map::sparse_cv_hashmap_t<tdc::dynamic_t>
// 1 = SeparateChainingTable -> separate_chaining_map
// else = SeparateChainingTable -> group::group_chaining_table
