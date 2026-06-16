#include "index_writer.h"
#include <iostream>
#include <cstring>
#include <chrono>

bool IndexWriter::open(const std::string& idx_path) 
{
    file_.open(idx_path, std::ios::binary | std::ios::trunc);
    if (!file_) 
    {
        std::cerr << "[index_writer] cannot open for write: " << idx_path << '\n';
        return false;
    }

    // Write a zeroed header as a placeholder.
    // finalise() will seek back and patch it hopefully :).
    LibraryHeader hdr{};
    file_.write(reinterpret_cast<const char*>(&hdr), sizeof(hdr));
    return true;
}

bool IndexWriter::add_track(const TrackRecord& rec) 
{
    if (!file_) return false;
    file_.write(reinterpret_cast<const char*>(&rec), sizeof(rec));
    ++track_count_;
    return true;
}

bool IndexWriter::finalise(uint32_t art_count) 
{
    if (!file_) return false;

    // Patch header at offset 0
    LibraryHeader hdr{};
    hdr.magic        = LIBRARY_MAGIC;
    hdr.version      = LIBRARY_VERSION;
    hdr.track_count  = track_count_;
    hdr.art_count    = art_count;
    hdr.generated_at = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count()
    );

    // reserved stays zero
    file_.seekp(0);
    file_.write(reinterpret_cast<const char*>(&hdr), sizeof(hdr));
    file_.flush();

    std::cout << "[index_writer] finalised — " << track_count_ << " tracks, " << art_count    << " art entries\n";
    return true;
}