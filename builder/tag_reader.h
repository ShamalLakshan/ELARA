#pragma once
#ifndef TAG_READER_H
#define TAG_READER_H

#include <string>
#include <vector>
#include <cstdint>

// Raw tags extracted from an audio file.
// All UTF-8. Empty string = tag absent.
struct AudioTags {
    std::string title;
    std::string artist;
    std::string album;
    uint32_t    duration_s = 0;

    // Raw embedded cover art bytes.
    // Empty vector = no art found.
    std::vector<uint8_t> art_data;
};

class TagReader {
public:
    // Read tags from an MP3 or FLAC file at path.
    // On false - out is left in a valid but empty state.
    bool read(const std::string& path, AudioTags& out) const;

private:
    // Extract the first APIC/PIC frame from ID3 or METADATA_BLOCK_PICTURE from FLAC Vorbis comments.
    std::vector<uint8_t> extract_art(const std::string& path) const;
};

#endif