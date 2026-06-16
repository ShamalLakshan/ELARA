#include "tag_reader.h"
#include "../lib/shared/include/library_types.h"

#include <iostream>
#include <fstream>
// TagLib headers
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/id3v2frame.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/flacfile.h>
#include <taglib/xiphcomment.h>
#include <taglib/flacpicture.h>

/*
=====================================================================================================
Helpers
=====================================================================================================
*/

static std::string tstring_to_utf8(const TagLib::String& s) 
{
    return s.to8Bit(true); // true = UTF-8
}

static void sanitise(std::string& s, size_t max_len) 
{
    // Remove null bytes that some taggers embed
    s.erase(std::remove(s.begin(), s.end(), '\0'), s.end());

    // Trim trailing whitespace
    while (!s.empty() && (s.back() == ' ' || s.back() == '\n' || s.back() == '\r'))
    {
        s.pop_back();
    }

    // Hard truncate to max_len - 1 to leave room for null terminator
    if (s.size() >= max_len)
    {
        s.resize(max_len - 1);
    }
}

/*
=====================================================================================================
Art Extraction
=====================================================================================================
*/

std::vector<uint8_t> TagReader::extract_art(const std::string& path) const {
    // figure out extension
    std::string ext;
    auto dot = path.rfind('.');

    if (dot != std::string::npos)
    {
        ext = path.substr(dot + 1);
    }
    for (auto& c : ext) c = static_cast<char>(tolower(c));

    if (ext == "mp3") 
    {
        TagLib::MPEG::File file(path.c_str());
        if (!file.isValid())
        {
            return {};
        } 

        auto* id3 = file.ID3v2Tag();
        if (!id3)
        {
            return {};
        }

        const auto& frames = id3->frameList("APIC");
        if (frames.isEmpty())
        {
            return {};
        }

        auto* apic = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(frames.front());
        if (!apic)
        {
            return {};
        }

        const auto& data = apic->picture();

        return std::vector<uint8_t>(data.data(), data.data() + data.size());
    }

    if (ext == "flac") 
    {
        TagLib::FLAC::File file(path.c_str());
        if (!file.isValid())
        {
            return {};
        } 

        const auto& pics = file.pictureList();
        if (pics.isEmpty())
        {
            return {};
        }

        const auto& data = pics.front()->data();
        return std::vector<uint8_t>(data.data(), data.data() + data.size());
    }

    return {};
}


/*
=====================================================================================================
Public Interface
=====================================================================================================
*/
bool TagReader::read(const std::string& path, AudioTags& out) const {
    out = {}; // reset

    TagLib::FileRef file(path.c_str(),
                      /*readAudioProperties=*/true,
                      TagLib::AudioProperties::Fast);

    if (file.isNull() || !file.tag()) 
    {
        std::cerr << "[tag_reader] cannot open: " << path << '\n';
        return false;
    }

    auto* tag = file.tag();

    out.title  = tstring_to_utf8(tag->title());
    out.artist = tstring_to_utf8(tag->artist());
    out.album  = tstring_to_utf8(tag->album());

    // Fallback: use filename stem as title if tag is empty
    if (out.title.empty()) 
    {
        auto stem = path.substr(path.rfind('/') + 1);
        auto dot  = stem.rfind('.');
        out.title = (dot != std::string::npos) ? stem.substr(0, dot) : stem;
    }
    if (out.artist.empty()) 
    {
        out.artist = "Unknown Artist";
    }
    if (out.album.empty())  
    {
        out.album  = "Unknown Album";
    }

    sanitise(out.title,  TITLE_LEN);
    sanitise(out.artist, ARTIST_LEN);
    sanitise(out.album,  ALBUM_LEN);

    // Duration
    if (file.audioProperties())
    {
        out.duration_s = static_cast<uint32_t>(file.audioProperties()->lengthInSeconds());
    }

    // Embedded art
    out.art_data = extract_art(path);

    return true;
}