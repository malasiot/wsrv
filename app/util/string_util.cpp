#include "string_util.hpp"

#include <random>

std::string randomString(size_t length) {
    // 1. Define the pool of available characters
    constexpr std::string_view alphabet = 
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    // 2. Initialize the pseudo-random number generator with a hardware seed
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<size_t> distribution(0, alphabet.size() - 1);

    // 3. Pre-allocate string space to avoid multi-pass heap memory reallocations
    std::string random_string;
    random_string.reserve(length);

    // 4. Fill the string container
    for (size_t i = 0; i < length; ++i) {
        random_string += alphabet[distribution(generator)];
    }

    return random_string;
}

std::string mimeFromHeader(const std::string& buffer) {
    if (buffer.size() < 4) {
        return "application/octet-stream"; // Fallback for unknown/tiny binary files
    }

    // Cast to unsigned char for easier hex comparison
    const auto* bytes = reinterpret_cast<const unsigned char*>(buffer.data());

    // 1. JPEG Check: Starts with FF D8 FF
    if (bytes[0] == 0xFF && bytes[1] == 0xD8 && bytes[2] == 0xFF) {
        return "image/jpeg";
    }

    // 2. PNG Check: Starts with 89 50 4E 47
    if (bytes[0] == 0x89 && bytes[1] == 0x50 && bytes[2] == 0x4E && bytes[3] == 0x47) {
        return "image/png";
    }

    // 3. GIF Check: Starts with 47 49 46 38 ("GIF8")
    if (bytes[0] == 0x47 && bytes[1] == 0x49 && bytes[2] == 0x46 && bytes[3] == 0x38) {
        return "image/gif";
    }

    // 4. WebP Check: Starts with "RIFF" and has "WEBP" at byte 8
    if (buffer.size() >= 12) {
        if (bytes[0] == 'R' && bytes[1] == 'I' && bytes[2] == 'F' && bytes[3] == 'F' &&
            bytes[8] == 'W' && bytes[9] == 'E' && bytes[10] == 'B' && bytes[11] == 'P') {
            return "image/webp";
        }
    }

    // 5. BMP Check: Starts with 42 4D ("BM")
    if (bytes[0] == 0x42 && bytes[1] == 0x4D) {
        return "image/bmp";
    }

    return "application/octet-stream"; // Default safety binary type
}