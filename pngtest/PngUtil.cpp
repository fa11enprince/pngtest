#include "PngUtil.h"
#include <sstream>
#include <fstream>

#define WIDTHBYTES(bits) (((bits)+31)/32*4)

void png_read_fnc(png_struct *png_ptr, png_byte *data, png_size_t length)
{
    DWORD readLen = 0;
    ::ReadFile(png_get_io_ptr(png_ptr), data, length, &readLen, NULL);
}

void PngUtil::Load(HWND hWnd, const tstring &fileName, bool isAlpha)
{
    PAINTSTRUCT ps;
    HDC hDC = BeginPaint(hWnd, &ps);
    HANDLE hFile;
    try
    {
        hFile = ::CreateFile(fileName.c_str(), GENERIC_READ, 0, NULL,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if ( hFile == INVALID_HANDLE_VALUE )
        {
            throw ExPngError("Cannot Open File");
        }
        png_byte sig[8];
        DWORD dwRead = 0;
        if ( ::ReadFile(hFile, sig, sizeof(sig), &dwRead, NULL) == 0 
            || !png_check_sig(sig, sizeof(sig)) )
        {
            throw ExPngError("Error occurred while reading");
        }
        // start reading png
        png_struct *png_ptr = png_create_read_struct(
            PNG_LIBPNG_VER_STRING,
            NULL,
            NULL,
            NULL);
        if (!png_ptr)
        {
            throw ExPngError("Error occurred in png_create_read_struct");
        }
        // initialize png_info struct
        png_info *info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr)
        {
            png_destroy_read_struct(&png_ptr, NULL, NULL);
            throw ExPngError("Error occurred in png_create_info_struct(info ptr)");
        }

        // assign io_ptr <- hFile
        png_set_read_fn(png_ptr,
            reinterpret_cast<png_voidp>(hFile),
            reinterpret_cast<png_rw_ptr>(png_read_fnc));
        // notify amount of loaded data
        png_set_sig_bytes(png_ptr, sizeof(sig)); 

        // READ HEADER =========================
        // get info header
        png_read_info(png_ptr, info_ptr);

        png_uint_32 width, height;
        int bit_depth, color_type;
        png_get_IHDR(
            png_ptr,
            info_ptr,
            &width,
            &height,
            &bit_depth,
            &color_type,
            NULL,
            NULL,
            NULL);

        int num_palette = 0;
        png_color *palette;
        if (color_type == PNG_COLOR_TYPE_PALETTE)
        {
            png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette);
        }

        if (bit_depth == 16)
        {
            png_set_strip_16(png_ptr);
        }

        // Windows default
        double screen_gamma = 2.2;
        double image_gamma;
        png_color_16p image_background;
        int intent;
        if (png_get_sRGB(png_ptr, info_ptr, &intent))
        {
            png_set_gamma( png_ptr, screen_gamma, 0.45455 );
        }
        else if (png_get_gAMA(png_ptr, info_ptr, &image_gamma))
        {
            if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
            {
                png_set_tRNS_to_alpha(png_ptr);
            }
            else if (!(info_ptr->color_type & PNG_COLOR_MASK_ALPHA))
            {
                png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
            }
            png_set_gamma(png_ptr, screen_gamma, image_gamma);
        }
 
        if (( info_ptr->color_type & PNG_COLOR_MASK_ALPHA ) 
             || png_get_valid( png_ptr, info_ptr, PNG_INFO_tRNS ))
        {
            if (png_get_bKGD( png_ptr, info_ptr, &image_background ))
            {
                png_set_background( png_ptr, image_background, PNG_BACKGROUND_GAMMA_FILE, 1, 1.0 );
            }
            else
            {
                png_color_16 my_background = { 0xff, 0xff, 0xff, 0xff, };
                png_set_background( png_ptr, &my_background, PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0 );
            }
        }

        // register and update png settings
        png_read_update_info(png_ptr, info_ptr);

        // bytes of row
        png_size_t row_bytes = png_get_rowbytes(png_ptr, info_ptr);
        // number of channel
        int channels = static_cast<int>(png_get_channels(png_ptr, info_ptr));

#ifdef _DEBUG
        std::ostringstream oss;
        oss << "size: " << width << " x " << height << "\n"
            << "bit_depth: " << bit_depth << "\n"
            << "color_type: " << color_type << "\n"
            << "row_bytes: " << row_bytes << "\n"
            << "channels: " << channels ;
        ::MessageBox(NULL, oss.str().c_str(), "", MB_OK);
#endif
        // allocate read data buffer
        png_byte *png_bytes_image = new png_byte[ row_bytes * height ]; 
        png_byte **png_bytes_row = new png_byte*[ height ];
        // read per 1 line
        for ( UINT i = 0; i < height; i++ )
        {
            png_bytes_row[i] = png_bytes_image + row_bytes * i;
        }

        // read image data
        png_read_image(png_ptr, png_bytes_row);
        // finallize reading
        png_read_end(png_ptr, NULL);

        // Create DIB
        DWORD dwSizeOfBitmapInfo=sizeof(BITMAPINFOHEADER) + num_palette * sizeof(RGBQUAD);
        BITMAPINFO* inf = reinterpret_cast<BITMAPINFO*>(new BYTE[dwSizeOfBitmapInfo]);
        ::ZeroMemory(inf, dwSizeOfBitmapInfo);

        int bit_count = 0;
        // if exists palette, copy the color table to BITMAPINFO
        if (color_type == PNG_COLOR_TYPE_PALETTE)
        {
            inf->bmiHeader.biBitCount = bit_depth;
            inf->bmiHeader.biClrUsed  = num_palette;
            for (int i = 0; i < num_palette; i++)
            {
                inf->bmiColors[i].rgbBlue  = palette[i].blue;
                inf->bmiColors[i].rgbGreen = palette[i].green;
                inf->bmiColors[i].rgbRed   = palette[i].red;
            }
        }
        else if ( channels == 3 )
        {
            bit_count = 24;
        }
        else if ( channels == 4 )
        {
            bit_count = 32;
        }
        DWORD dwWidthBytes = WIDTHBYTES(bit_count * width);

        inf->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        inf->bmiHeader.biWidth = width;
        inf->bmiHeader.biHeight = height;
        inf->bmiHeader.biPlanes = 1;
        inf->bmiHeader.biBitCount = bit_count;
        inf->bmiHeader.biCompression = BI_RGB;
        inf->bmiHeader.biSizeImage = dwWidthBytes * height;

        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

        // Create DIBSection
        LPVOID pvBits;
        HBITMAP hBitmap = ::CreateDIBSection(
            hDC,
            inf,
            DIB_RGB_COLORS,
            &pvBits,
            NULL,
            0);
        if ( !hBitmap )
        {
            delete[] png_bytes_row;
            throw ExPngError("Error occurred in create DIBSection)");
        }

        CopyImageData(
            channels,
            height,
            width,
            row_bytes,
            bit_depth,
            dwWidthBytes,
            png_bytes_image,
            pvBits);

        HDC hMemDc = ::CreateCompatibleDC(hDC);
        SelectObject(hMemDc, hBitmap);

        ::BitBlt(
            hDC,    // hdcDest
            0,      // nXDest
            0,      // nYDest
            width,  // nWidth
            height, // nHeight
            hMemDc, // hdcSrc
            0,      // nXSrc
            0,      // nYSrc
            SRCCOPY // raster-operation code
            );

        // finallize Display image
        ::DeleteDC(hMemDc);
        ::DeleteObject(hBitmap);
        ::ReleaseDC(hWnd, hDC);

        ::CloseHandle(hFile);
        delete[] inf;
        delete[] png_bytes_row;
    }   
    catch(ExPngError &e)
    {
        ::CloseHandle(hFile);
        std::cerr << e.what() << std::endl;
    }
    EndPaint(hWnd, &ps);
}

void PngUtil::CopyImageData(int channels, png_uint_32 height, png_uint_32 width,
                            png_size_t row_bytes, int bit_depth, DWORD dwWidthBytes, 
                            png_byte *png_bytes_image, LPVOID pvBits)
{
    switch(channels)
    {
    case 1: // palette DIB
        for (UINT i = 0; i < height; i++)
        {
            CopyMemory(
            reinterpret_cast<BYTE*>(pvBits) + i * dwWidthBytes,
                png_bytes_image + (height - 1 - i) * row_bytes,
                (width * bit_depth + 7) / 8);
        }
        break;
    case 3: // 24 bit RGBDIB
        for (UINT i = 0; i < height; i++)
        {
            BYTE *src = png_bytes_image +(height - 1 - i) * row_bytes;
            BYTE *dst = static_cast<BYTE*>(pvBits) + i * dwWidthBytes;
            for(UINT j = 0; j < width; j++ )
            {
                *dst++ = src[2];  // Blue
                *dst++ = src[1];  // Green
                *dst++ = src[0];  // Red
                src += 3;
            }
        }
        break;
     case 4: // 32 bit + alpha channel
        BYTE r, g, b, a;
        for (UINT i = 0; i < height; i++)
        {
            BYTE *src = png_bytes_image + (height - 1 - i) * row_bytes;
            BYTE *dst = static_cast<BYTE*>(pvBits) + i * dwWidthBytes;
            for(UINT j = 0; j < width; j++ )
            {
                r = *src++;  // Red
                g = *src++;  // Green 
                b = *src++;  // Blue
                a = *src++;  // alpha
                *dst++ = b;
                *dst++ = g;
                *dst++ = r;
                *dst++ = a;
            }
        }
        break;
    }
}