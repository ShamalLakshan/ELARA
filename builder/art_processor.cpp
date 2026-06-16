#include "art_processor.h"
#include <iostream>
#include <fstream>
#include <cstring>

// stb_image — single-header image decode
#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "../lib/stb/stb_image.h"

// stb_image_resize2 — single-header resize
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "../lib/stb/stb_image_resize2.h"

// stb_image_write — single-header JPEG encode
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../lib/stb/stb_image_write.h"

static constexpr int kThumbSize    = 64;
static constexpr int kJpegQuality  = 85;
static constexpr int kChannels     = 3; // RGB - no alpha

/*
=====================================================================================================
FNV-1a 64-bit hash (no external dep)
=====================================================================================================
*/
uint64_t ArtProcessor::hash_bytes(const std::vector<uint8_t>& data) {
    constexpr uint64_t FNV_OFFSET = 14695981039346656037ULL;
    constexpr uint64_t FNV_PRIME  = 1099511628211ULL;
    uint64_t h = FNV_OFFSET;
    for (uint8_t b : data) 
    {
        h ^= b;
        h *= FNV_PRIME;
    }
    return h;
}

/*
=====================================================================================================
Placeholder grey title
=====================================================================================================
*/
void ArtProcessor::ensure_placeholder() {
    if (!entries_.empty()) 
    {
        return;
    }

    // 64x64 mid-grey RGB
    std::vector<uint8_t> grey(kThumbSize * kThumbSize * kChannels, 0x88);

    // Encode to JPEG in memory
    std::vector<uint8_t> jpeg;
    stbi_write_jpg_to_func([](void* ctx, void* data, int size) 
        {
            auto* v = reinterpret_cast<std::vector<uint8_t>*>(ctx);
            const uint8_t* p = reinterpret_cast<uint8_t*>(data);
            v->insert(v->end(), p, p + size);
        },
        &jpeg, kThumbSize, kThumbSize, kChannels,
        grey.data(), kJpegQuality
    );

    ArtEntry placeholder;
    placeholder.data   = std::move(jpeg);
    placeholder.size   = static_cast<uint32_t>(placeholder.data.size());
    placeholder.offset = 0;

    uint64_t h = hash_bytes(placeholder.data);
    hash_to_index_[h] = 0;
    entries_.push_back(std::move(placeholder));
}

/*
=====================================================================================================
Process
=====================================================================================================
*/
uint32_t ArtProcessor::process(const std::vector<uint8_t>& raw) 
{
    ensure_placeholder();

    if (raw.empty()) 
    {
        return 0;
    }

    // Hash the raw bytes for deduplication before decode
    uint64_t h = hash_bytes(raw);
    auto it = hash_to_index_.find(h);
    if (it != hash_to_index_.end())
    {
        return it->second;
    }

    // Decode
    int w = 0, h_px = 0, ch = 0;
    uint8_t* pixels = stbi_load_from_memory(raw.data(), static_cast<int>(raw.size()),&w, &h_px, &ch, kChannels);
    if (!pixels) 
    {
        std::cerr << "[art_processor] decode failed: " << stbi_failure_reason() << '\n';
        return 0;
    }

    // Resize to 64x64
    std::vector<uint8_t> thumb(kThumbSize * kThumbSize * kChannels);
    stbir_resize_uint8_linear(
        pixels, w, h_px, 0,
        thumb.data(), kThumbSize, kThumbSize, 0,
        (stbir_pixel_layout)kChannels
    );
    stbi_image_free(pixels);

    // Encode to JPEG in memory
    std::vector<uint8_t> jpeg;
    jpeg.reserve(4096);
    stbi_write_jpg_to_func([](void* ctx, void* data, int size) 
        {
            auto* v = reinterpret_cast<std::vector<uint8_t>*>(ctx);
            const uint8_t* p = reinterpret_cast<uint8_t*>(data);
            v->insert(v->end(), p, p + size);
        },
        &jpeg, kThumbSize, kThumbSize, kChannels,
        thumb.data(), kJpegQuality
    );

    uint32_t idx = static_cast<uint32_t>(entries_.size());
    hash_to_index_[h] = idx;

    ArtEntry entry;
    entry.data   = std::move(jpeg);
    entry.size   = static_cast<uint32_t>(entry.data.size());
    entry.offset = 0; // filled in during write()
    entries_.push_back(std::move(entry));

    return idx;
}


/*
=====================================================================================================
Write
=====================================================================================================
*/
bool ArtProcessor::write(const std::string& out_path) 
{
    ensure_placeholder();

    std::ofstream f(out_path, std::ios::binary | std::ios::trunc);
    if (!f) 
    {
        std::cerr << "[art_processor] cannot write: " << out_path << '\n';
        return false;
    }

    uint32_t offset = 0;
    for (auto& entry : entries_) 
    {
        entry.offset = offset;
        f.write(reinterpret_cast<const char*>(entry.data.data()),
                static_cast<std::streamsize>(entry.data.size()));
        offset += entry.size;
    }

    std::cout << "[art_processor] wrote " << entries_.size() << " art entries to " << out_path << '\n';
    return true;
}