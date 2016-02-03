#pragma once
#include <Windows.h>
#include <iostream>
#include <exception>
#include <png.h>
#include <pngstruct.h>
#include <pnginfo.h>

typedef std::basic_string<TCHAR> tstring;

class PngUtil
{
public:
    void    Load(HWND hWnd, const tstring &fileName, bool isAlpha=false);
private:
    void    CopyImageData(
                int channels, png_uint_32 height, png_uint_32 width,
                png_size_t row_bytes, int bit_depth, DWORD dwWidthBytes, 
                png_byte *png_bytes_image, LPVOID pvBits);
};

class ExPngError : public std::exception
{
public:
    ExPngError(const char* const &message)
    {
        pngError_ = message;
    }
    const char *what() const override
    {
        return pngError_;
    }
protected:
    const char* pngError_;
};


