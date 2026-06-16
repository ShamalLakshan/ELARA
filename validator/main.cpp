#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstring>

#include "../lib/shared/include/library_types.h"

// namespace std::filesystem = std::filesystem;

static bool validate(const std::string& idx_path, const std::string& art_path) 
{
    /*
    =====================================================================================================
    Open Files
    =====================================================================================================
    */
    std::ifstream idx(idx_path, std::ios::binary);
    if (!idx) 
    {
        std::cerr << "[validator] cannot open: " << idx_path << '\n';
        return false;
    }

    uint64_t art_size = 0;
    if (std::filesystem::exists(art_path)) 
    {
        art_size = std::filesystem::file_size(art_path);
    } else 
    {
        std::cerr << "[validator] art.bin not found: " << art_path << '\n';
        return false;
    }

    /*
    =====================================================================================================
    Header
    =====================================================================================================
    */
    LibraryHeader hdr{};
    idx.read(reinterpret_cast<char*>(&hdr), sizeof(hdr));
    if (!idx) 
    {
        std::cerr << "[validator] file too small to contain header\n";
        return false;
    }

    bool ok = true;

    if (hdr.magic != LIBRARY_MAGIC) 
    {
        std::cerr << "[validator] FAIL magic: expected 0x" << std::hex << LIBRARY_MAGIC
                << " got 0x" << hdr.magic << std::dec << '\n';
        ok = false;
    } else {
        std::cout << "[validator] magic        OK\n";
    }

    if (hdr.version != LIBRARY_VERSION) 
    {
        std::cerr << "[validator] FAIL version: expected " << LIBRARY_VERSION << " got " << hdr.version << '\n';
        ok = false;
    } 
    else
    {
        std::cout << "[validator] version      OK (" << hdr.version << ")\n";
    }

    std::cout << "[validator] track_count  " << hdr.track_count << '\n';
    std::cout << "[validator] art_count    " << hdr.art_count   << '\n';
    std::cout << "[validator] generated_at " << hdr.generated_at << '\n';

    /*
    =====================================================================================================
    Expected file size
    =====================================================================================================
    */
    uint64_t expected_size = sizeof(LibraryHeader) + static_cast<uint64_t>(hdr.track_count) * sizeof(TrackRecord);
    uint64_t actual_size   = std::filesystem::file_size(idx_path);

    if (actual_size != expected_size) 
    {
        std::cerr << "[validator] FAIL size: expected " << expected_size << " bytes, got " << actual_size << '\n';
        ok = false;
    } 
    else 
    {
        std::cout << "[validator] file size    OK (" << actual_size << " bytes)\n";
    }

    /*
    =====================================================================================================
    Per Record Checks
    =====================================================================================================
    */
    uint32_t bad_records   = 0;
    uint32_t no_title      = 0;
    uint32_t art_oob       = 0;

    for (uint32_t i = 0; i < hdr.track_count; ++i) 
    {
        TrackRecord rec{};
        idx.read(reinterpret_cast<char*>(&rec), sizeof(rec));
        if (!idx) 
        {
            std::cerr << "[validator] FAIL unexpected EOF at record " << i << '\n';
            ok = false;
            break;
        }

        // ID should match position
        if (rec.id != i) 
        {
            if (bad_records < 5)
                std::cerr << "[validator] FAIL id mismatch record " << i << ": id=" << rec.id << '\n';
            ++bad_records;
            ok = false;
        }

        // Title must not be empty
        if (rec.title[0] == '\0') ++no_title;

        // Path must start with '/'
        if (rec.path[0] != '/') 
        {
            if (bad_records < 5)
            {
                std::cerr << "[validator] FAIL bad path at record " << i << ": " << rec.path << '\n';
            }
            ++bad_records;
            ok = false;
        }

        // Art offset + size must not exceed art.bin size
        if (rec.art_size > 0) 
        {
            uint64_t end = static_cast<uint64_t>(rec.art_offset) + rec.art_size;
            if (end > art_size) 
            {
                ++art_oob;
                ok = false;
            }
        }
    }

    if (bad_records > 0)
    {
        std::cerr << "[validator] FAIL " << bad_records << " bad records\n";

    }
    else
    {
        std::cout << "[validator] records      OK\n";

    }

    if (no_title > 0)
    {
        std::cout << "[validator] warn: " << no_title << " tracks have no title\n";

    }

    if (art_oob > 0)
    {
        std::cerr << "[validator] FAIL " << art_oob << " records have art offset beyond art.bin\n";

    }
    else
    {
        std::cout << "[validator] art offsets  OK\n";

    }

    std::cout << "[validator] result: " << (ok ? "PASS" : "FAIL") << '\n';
    return ok;
}

int main(int argc, char* argv[]) 
{
    if (argc != 3) 
    {
        std::cerr << "Usage: " << argv[0] << " <library.idx> <art.bin>\n";
        return 1;
    }
    return validate(argv[1], argv[2]) ? 0 : 1;
}
