#pragma once
#ifndef ART_PROCESSOR_H
#define ART_PROCESSOR_H

#include <vector>
#include <cstdint>
#include <string>
#include <unordered_map>

// A single deduplicated art entry 
// ready to write into art.bin.
struct ArtEntry {
    uint32_t             offset = 0;   // byte offset in art.bin
    uint32_t             size   = 0;   // byte size of JPEG thumbnail
    std::vector<uint8_t> data;         // 64x64 JPEG bytes
};

class ArtProcessor {
public:
    // Decode raw (JPEG or PNG bytes), resize to 64x64,
    // re-encode as JPEG quality 85, and deduplicate.
    // Returns the index into entries() for this image.
    // If raw is empty or decode fails : returns 0
    // (index 0 is always the placeholder image).
    uint32_t process(const std::vector<uint8_t>& raw);

    // All unique art entries accumulated 
    // Entry 0 is always the grey placeholder.
    const std::vector<ArtEntry>& entries() const { return entries_; }

    // Write art.bin to disk. Updates offset fields in entries
    bool write(const std::string& out_path);

private:
    void ensure_placeholder();

    // xxHash-style 64-bit FNV1a over the raw bytes
    // fast enough for dedup, no external dependency.
    static uint64_t hash_bytes(const std::vector<uint8_t>& data);

    std::vector<ArtEntry> entries_;
    std::unordered_map<uint64_t, uint32_t> hash_to_index_;
};

#endif