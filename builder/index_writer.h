#pragma once
#ifndef INDEX_WRITER_H
#define INDEX_WRITER_H

#include "../lib/shared/include/library_types.h"
#include "art_processor.h"
#include <string>
#include <vector>
#include <fstream>

class IndexWriter {
public:
    // Open idx_path for writing.
    // Call add_track() for each file, then finalise().
    bool open(const std::string& idx_path);

    // Append one track record.
    // art_entry_idx is the index returned by ArtProcessor::process().
    // art_processor is passed to resolve offset/size at write time.
    bool add_track(const TrackRecord& rec);

    // Patch the header with final counts and flush.
    // Must be called after all add_track() calls.
    bool finalise(uint32_t art_count);

    uint32_t track_count() const { return track_count_; }

private:
    std::ofstream file_;
    uint32_t      track_count_ = 0;
};

#endif