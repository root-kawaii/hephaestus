#include <fstream>
#include <iostream>
#include "json/single_include/json.hpp"
#include "lz4.h"

namespace assets
{
    struct AssetFile
    {
        char type[4];
        int version;
        std::string json;
        std::vector<char> binaryBlob;
    };

    bool save_binaryfile(const char *path, const AssetFile &file);

    bool load_binaryfile(const char *path, AssetFile &outputFile);

    enum class TextureFormat : uint32_t
    {
        Unknown = 0,
        RGBA8
    };

    enum class CompressionMode : uint32_t
    {
        None,
        LZ4
    };

    struct TextureInfo
    {
        uint64_t textureSize;
        TextureFormat textureFormat;
        CompressionMode compressionMode;
        uint32_t pixelsize[3];
        std::string originalFile;
    };

    // parses the texture metadata from an asset file
    TextureInfo read_texture_info(AssetFile *file);

    void unpack_texture(TextureInfo *info, const char *sourcebuffer, size_t sourceSize, char *destination);

    AssetFile pack_texture(TextureInfo *info, void *pixelData);

}

bool assets::save_binaryfile(const char *path, const assets::AssetFile &file)
{
    std::ofstream outfile;
    outfile.open(path, std::ios::binary | std::ios::out);

    outfile.write(file.type, 4);
    uint32_t version = file.version;
    // version
    outfile.write((const char *)&version, sizeof(uint32_t));

    // json length
    uint32_t length = file.json.size();
    outfile.write((const char *)&length, sizeof(uint32_t));

    // blob length
    uint32_t bloblength = file.binaryBlob.size();
    outfile.write((const char *)&bloblength, sizeof(uint32_t));

    // json stream
    outfile.write(file.json.data(), length);
    // blob data
    outfile.write(file.binaryBlob.data(), file.binaryBlob.size());

    outfile.close();

    return true;
}

bool assets::load_binaryfile(const char *path, assets::AssetFile &outputFile)
{
    std::ifstream infile;
    infile.open(path, std::ios::binary);

    if (!infile.is_open())
        return false;

    // move file cursor to beginning
    infile.seekg(0);

    infile.read(outputFile.type, 4);
    infile.read((char *)&outputFile.version, sizeof(uint32_t));

    uint32_t jsonlen = 0;
    infile.read((char *)&jsonlen, sizeof(uint32_t));

    uint32_t bloblen = 0;
    infile.read((char *)&bloblen, sizeof(uint32_t));

    outputFile.json.resize(jsonlen);
    infile.read(outputFile.json.data(), jsonlen);

    outputFile.binaryBlob.resize(bloblen);
    infile.read(outputFile.binaryBlob.data(), bloblen);

    return true;
}

assets::AssetFile assets::pack_texture(assets::TextureInfo *info, void *pixelData)
{
    nlohmann::json texture_metadata;
    texture_metadata["format"] = "RGBA8";
    texture_metadata["width"] = info->pixelsize[0];
    texture_metadata["height"] = info->pixelsize[1];
    texture_metadata["buffer_size"] = info->textureSize;
    texture_metadata["original_file"] = info->originalFile;

    // core file header
    AssetFile file;
    file.type[0] = 'T';
    file.type[1] = 'E';
    file.type[2] = 'X';
    file.type[3] = 'I';
    file.version = 1;

    // compress buffer into blob
    int compressStaging = LZ4_compressBound(info->textureSize);

    file.binaryBlob.resize(compressStaging);

    int compressedSize = LZ4_compress_default((const char *)pixelData, file.binaryBlob.data(), info->textureSize, compressStaging);

    file.binaryBlob.resize(compressedSize);

    texture_metadata["compression"] = "LZ4";

    std::string stringified = texture_metadata.dump();
    file.json = stringified;

    return file;
}

void assets::unpack_texture(TextureInfo *info, const char *sourcebuffer, size_t sourceSize, char *destination)
{
    if (info->compressionMode == CompressionMode::LZ4)
    {
        LZ4_decompress_safe(sourcebuffer, destination, sourceSize, info->textureSize);
    }
    else
    {
        memcpy(destination, sourcebuffer, sourceSize);
    }
}