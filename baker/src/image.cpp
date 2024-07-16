#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <jpeglib.h>
#include <png.h>
#include <iostream>
#include <webp/encode.h>
#include <webp/decode.h>
#include <webp/mux.h>

/*
- In dev -

Compile: g++ -o image baker/src/image.cpp -lwebp -ljpeg -lpng

Support:
    (Lossy)
    - .png/apng (~95%)
    - .webp (~20%)
    - .jpg/jpeg (~80%)

    (Ultra)
    - .png/apng (~99%)
    - .webp (~40%)
    - .jpg/jpeg (~96%)

Wip:
    - .svg

Not supported yet:
    - .gif
    - .avif
*/

const char* humanizeSize(std::size_t size) {
    static const char* suffixes[] = {"bytes", "kB", "mB", "gB", "tB"};
    int suffixIndex = 0;
    double doubleSize = static_cast<double>(size);

    while (doubleSize >= 1024 && suffixIndex < sizeof(suffixes) / sizeof(suffixes[0]) - 1) {
        suffixIndex++;
        doubleSize /= 1024;
    }

    char buffer[50]; // Assuming size won't exceed 50 characters
    std::snprintf(buffer, sizeof(buffer), "%.2f %s", doubleSize, suffixes[suffixIndex]);
    
    return std::string(buffer).c_str();
}

uint8_t* LoadFile(const char* file_name, std::size_t* file_size) {
    FILE* file = fopen(file_name, "rb");
    if (!file)
        return nullptr;
    fseek(file, 0, SEEK_END);
    *file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    uint8_t* data = (uint8_t*)malloc(*file_size);
    if (!data) {
        fclose(file);
        return nullptr;
    }
    fread(data, *file_size, 1, file);
    fclose(file);
    return data;
}

// Function to save memory buffer into an image file
int SaveFile(const char* file_name, const uint8_t* data, std::size_t file_size) {
    FILE* file = fopen(file_name, "wb");
    if (!file)
        return 0;
    fwrite(data, file_size, 1, file);
    fclose(file);
    return 1;
}

uint8_t* DecodePNG(const uint8_t* data, std::size_t data_size, int* width, int* height) {
    png_image image;
    memset(&image, 0, (sizeof image));
    image.version = PNG_IMAGE_VERSION;
    if (!png_image_begin_read_from_memory(&image, data, data_size))
        return nullptr;
    image.format = PNG_FORMAT_RGBA;
    *width = image.width;
    *height = image.height;
    uint8_t* raw_image = (uint8_t*)malloc(PNG_IMAGE_SIZE(image));
    if (!png_image_finish_read(&image, nullptr, raw_image, 0, nullptr)) {
        free(raw_image);
        return nullptr;
    }
    return raw_image;
}

uint8_t* DecodeJPEG(const uint8_t* data, std::size_t data_size, int* width, int* height) {
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);

    // Initialize decompression object
    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, data, data_size);

    // Read JPEG header
    if (jpeg_read_header(&cinfo, TRUE) != 1) {
        jpeg_destroy_decompress(&cinfo);
        return nullptr;
    }

    // Start decompression
    jpeg_start_decompress(&cinfo);
    *width = cinfo.output_width;
    *height = cinfo.output_height;
    int pixel_size = cinfo.output_components;

    // Allocate memory for the output RGBA image
    uint8_t* raw_image = (uint8_t*)malloc(*width * *height * 4);
    if (!raw_image) {
        jpeg_destroy_decompress(&cinfo);
        return nullptr;
    }

    // Allocate buffer for one row (scanline)
    uint8_t* buffer = (uint8_t*)malloc(cinfo.output_width * pixel_size);

    if (!buffer) {
        free(raw_image);
        jpeg_destroy_decompress(&cinfo);
        return nullptr;
    }

    // Pointer to the beginning of the output image buffer
    uint8_t* ptr = raw_image;

    // Read scanlines one by one and copy pixel data
    while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, &buffer, 1);
        for (int x = 0; x < cinfo.output_width; x++) {
            ptr[0] = buffer[x * pixel_size];
            ptr[1] = buffer[x * pixel_size + 1];
            ptr[2] = buffer[x * pixel_size + 2];
            ptr[3] = 255; // Set alpha channel to opaque
            ptr += 4;     // Move to the next pixel in RGBA buffer
        }
    }

    // Cleanup
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    free(buffer); // Free the scanline buffer

    return raw_image;
}

int main(int argc, char* argv[]) {
    if (argc != 3 && argc != 4) {
        printf("Usage: %s input_image output_image.webp\n", argv[0]);
        return 1;
    }

    const char* input_file = argv[1];
    const char* output_file = argv[2];

    // Load the input image
    std::size_t data_size;
    uint8_t* data = LoadFile(input_file, &data_size);
    if (!data) {
        fprintf(stderr, "Failed to read file: %s\n", input_file);
        return 1;
    }
    printf("Input file size: %s\n", humanizeSize(data_size));

    // Decode the image
    int width, height;
    uint8_t* rgba = 0;
    if (strstr(input_file, ".jpg") || strstr(input_file, ".jpeg")) {
        rgba = DecodeJPEG(data, data_size, &width, &height);
    } else if (strstr(input_file, ".png") || strstr(input_file, ".apng")) {
        rgba = DecodePNG(data, data_size, &width, &height);
    } else if (strstr(input_file, ".webp")) {
        // Decode WebP image
        rgba = WebPDecodeRGBA(data, data_size, &width, &height);
        if (!rgba) {
            fprintf(stderr, "Failed to decode WebP image: %s\n", input_file);
            free(data);
            return 1;
        }
    }
    free(data);

    if (!rgba) {
        fprintf(stderr, "Failed to decode image: %s\n", input_file);
        return 1;
    }

    // Encode to WebP
    uint8_t* output_data = nullptr;

    // Lossless: 100
    // Lossy: 30
    // Ultra: 10
    int quality;
    if (argc == 4) {
        quality = std::stoi(argv[3]);
    } else {
        quality = 30;
    }
    std::size_t output_size = WebPEncodeRGBA(rgba, width, height, width * 4, quality, &output_data);
    free(rgba);
    if (!output_size) {
        fprintf(stderr, "Failed to encode to WebP.\n");
        return 1;
    }

    // Save the output WebP file
    if (!SaveFile(output_file, output_data, output_size)) {
        fprintf(stderr, "Failed to save WebP file: %s\n", output_file);
        WebPFree(output_data);
        return 1;
    }
    printf("Output file size: %s\n", humanizeSize(output_size));
    printf("Compression: -%.2f%%\n", (1.0 - (double)output_size / data_size) * 100.0);

    WebPFree(output_data);
    printf("Successfully converted to WebP: %s\n", output_file);
    return 0;
}
