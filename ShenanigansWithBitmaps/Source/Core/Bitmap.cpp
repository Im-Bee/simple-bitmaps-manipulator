#include "Pch.h"

#include "Bitmap.hpp"

using namespace SWBitmaps;


// Bitmap ----------------------------------------------------------------------

// -----------------------------------------------------------------------------
void Bitmap::Initialize(IN const std::wstring& path)
{
    m_Path = path;

    LoadFromPath();
    if (!m_Header.Valid)
        return;
    ReadHeader();
    if (!m_Header.Valid)
        return;

    MapImage();
}

// -----------------------------------------------------------------------------
void Bitmap::Destroy()
{
    m_MappedImage.Clear();

    if (m_ImageBuff != nullptr)
    {
        free(m_ImageBuff);
        m_ImageBuff = nullptr;
    }
}

// -----------------------------------------------------------------------------
void Bitmap::SaveToFile(IN const std::wstring& path)
{
    std::ofstream file(path,
        std::ios_base::binary | std::ios_base::out);

    if (!file.is_open())
    {
        m_Header.Valid = false;
        return;
    }

    for (uint64_t i = 0; i < m_uSizeOfBuff; i += BITMAP_CHUNK)
        file.write((m_ImageBuff + i), BITMAP_CHUNK);

    file.close();
}

// Image manipulation ----------------------------------------------------------

// -----------------------------------------------------------------------------
#define SWB_FOR_WHOLE_IMAGE_I_K                                 \
for (uint64_t i = 0; i < m_MappedImage.GetHeight(); i++)        \
{                                                               \
    for (uint64_t k = 0; k < m_MappedImage.GetWidth(i); k++)    \
    {

#define SWB_FOR_WHOLE_IMAGE_I_K_END }}

// -----------------------------------------------------------------------------
void SWBitmaps::Bitmap::ScaleTo(uint32_t width, uint32_t height)
{
    char* originalBuf = m_ImageBuff;
    PixelMapWrapper originalMap = m_MappedImage;
    m_MappedImage.Clear();

    // If no height than scale with aspect ratio
    if (!height)
        height = (width * ((long double)m_Header.Height / m_Header.Width));

    // Pre calculate
    const long double fNewHeightRatio = (long double)m_Header.Height / height;
    const long double fNewWidthRatio = (long double)m_Header.Width / width;

    m_Header.Width = width;
    m_Header.Height = height;
#pragma warning ( push )
#pragma warning ( disable : 4244 )
    const uint64_t calcWidth = std::floorl((((m_Header.ColorDepth * m_Header.Width) + 31) / 32)) * 4;
#pragma warning ( pop )
    m_Header.FileSize = (calcWidth * m_Header.Height) + m_Header.FileBeginOffset;
    m_Header.ImageSize = (calcWidth * m_Header.Height);
    m_uSizeOfBuff = sizeof(char) * m_Header.FileSize;
    m_ImageBuff = (char*)malloc(m_uSizeOfBuff);
    MakeHeader();
    MapImage();

    for (int64_t i = 0; i < height; i++)
    {
        for (int64_t k = 0; k < width; k++)
        {
            auto& p = originalMap.Pixel(static_cast<size_t>(i * fNewHeightRatio),
                static_cast<size_t>(k * fNewWidthRatio));

            m_MappedImage.Pixel(i, k).Red() = 0 + p.Red();
            m_MappedImage.Pixel(i, k).Blue() = 0 + p.Blue();
            m_MappedImage.Pixel(i, k).Green() = 0 + p.Green();
        }
    }

    free(originalBuf);
    originalMap.~PixelMapWrapper();
}

// -----------------------------------------------------------------------------
void Bitmap::ColorWhole(IN Color c)
{
    SWB_FOR_WHOLE_IMAGE_I_K;
        if (MappedPixel::IsInvalid(m_MappedImage.Pixel(i, k)))
            continue;

        auto& p = m_MappedImage.Pixel(i, k);

        p.Red() = c.Red;
        p.Green() = c.Green;
        p.Blue() = c.Blue;
    SWB_FOR_WHOLE_IMAGE_I_K_END;
}

// -----------------------------------------------------------------------------
void SWBitmaps::Bitmap::ColorHalf(IN Color c)
{
    for (uint64_t i = 0; i < m_MappedImage.GetHeight() / 2; i++)
    {
        for (uint64_t k = 0; k < m_MappedImage.GetWidth(i); k++)
        {
            if (MappedPixel::IsInvalid(m_MappedImage.Pixel(i, k)))
                continue;

            auto& p = m_MappedImage.Pixel(i, k);

            p.Red() = c.Red;
            p.Green() = c.Green;
            p.Blue() = c.Blue;
        }
    }
}

// -----------------------------------------------------------------------------
void Bitmap::MakeItRainbow()
{
#pragma warning ( push )
#pragma warning ( disable : 4244 )
    srand(time(NULL));
#pragma warning ( pop )

    SWB_FOR_WHOLE_IMAGE_I_K
        if (MappedPixel::IsInvalid(m_MappedImage.Pixel(i, k)))
            continue;
        
        auto& p = m_MappedImage.Pixel(i, k);
        
        p.Red() = std::rand() % 256;
        p.Green() = std::rand() % 256;
        p.Blue() = std::rand() % 256;
    SWB_FOR_WHOLE_IMAGE_I_K_END
}

// -----------------------------------------------------------------------------
void Bitmap::MakeItNegative()
{
    SWB_FOR_WHOLE_IMAGE_I_K
        if (MappedPixel::IsInvalid(m_MappedImage.Pixel(i, k)))
            continue;
        
        auto& p = m_MappedImage.Pixel(i, k);
        
        p.Red() = 255 - p.Red();
        p.Green() = 255 - p.Green();
        p.Blue() = 255 - p.Blue();
    SWB_FOR_WHOLE_IMAGE_I_K_END
}

// -----------------------------------------------------------------------------
void SWBitmaps::Bitmap::MakeItGrayScale()
{
    SWB_FOR_WHOLE_IMAGE_I_K;
        if (MappedPixel::IsInvalid(m_MappedImage.Pixel(i, k)))
            continue;

        auto& p = m_MappedImage.Pixel(i, k);
        uint8_t average = (static_cast<uint32_t>(p.Red()) + p.Green() + p.Blue()) / 3;

        p.Red() = average;
        p.Green() = average;
        p.Blue() = average;
    SWB_FOR_WHOLE_IMAGE_I_K_END;
}

// -----------------------------------------------------------------------------
void SWBitmaps::Bitmap::DeleteShadows()
{
    
}

// Private ---------------------------------------------------------------------

// -----------------------------------------------------------------------------
void Bitmap::LoadFromPath()
{
    if (m_Path.find(L'.') != std::wstring::npos)
    {
        wchar_t fileExt[8];
        wcscpy_s(fileExt, m_Path.substr(m_Path.find_last_of('.')).c_str());
        _wcslwr_s(fileExt);
        if (!wcscmp(fileExt, L"bmp"))
            return;
    }

    std::ifstream file(m_Path,
        std::ios_base::binary | std::ios_base::in | std::ios_base::ate);

    if (!file.is_open())
    {
        m_Header.Valid = false;
        return;
    }
 
    uint64_t endOfFile = file.tellg();

    m_uSizeOfBuff = sizeof(char) * endOfFile;
    m_ImageBuff = (char*) malloc(m_uSizeOfBuff);

    file.seekg(std::ios_base::beg);

    for (uint64_t i = 0; !file.eof(); i++)
        file.read((m_ImageBuff + file.tellg()), BITMAP_CHUNK);

    file.close();

#ifdef _DEBUG
    // PrintFirstChunk();
#endif // _DEBUG

    m_Header.Valid = true;
}

#define CAST_READ_JUMP(loadTo, dataType, buffer, jumpVal)   \
loadTo = *((dataType*)&buffer[jumpVal]);                    \
jumpVal += sizeof(dataType);

// -----------------------------------------------------------------------------
void Bitmap::ReadHeader()
{
    if (m_ImageBuff[0] != 'B' ||
        m_ImageBuff[1] != 'M' ||
        m_uSizeOfBuff < 14)
        return;
    else
        m_Header.Valid = true;


    m_Header.FileSize = *((uint32_t*)(&m_ImageBuff[2]));
    m_Header.FileBeginOffset = *((uint32_t*)&m_ImageBuff[10]);

    if (m_Header.FileBeginOffset == BITMAPINFOHEADER)
    {
        uint8_t jump = 14;
        CAST_READ_JUMP(m_Header.SizeOfHeader, uint32_t, m_ImageBuff, jump);
        CAST_READ_JUMP(m_Header.Width, int32_t, m_ImageBuff, jump);
        CAST_READ_JUMP(m_Header.Height, int32_t, m_ImageBuff, jump);
        CAST_READ_JUMP(m_Header.ColorPlanes, uint16_t, m_ImageBuff, jump);
        CAST_READ_JUMP(m_Header.ColorDepth, uint16_t, m_ImageBuff, jump);
        CAST_READ_JUMP(m_Header.CompressionMethod, uint32_t, m_ImageBuff, jump);
        CAST_READ_JUMP(m_Header.ImageSize, uint32_t, m_ImageBuff, jump);
        CAST_READ_JUMP(m_Header.HorizontalResolution, int32_t, m_ImageBuff, jump);
        CAST_READ_JUMP(m_Header.VerticalResolution, int32_t, m_ImageBuff, jump);
        CAST_READ_JUMP(m_Header.ColorsInPalete, uint32_t, m_ImageBuff, jump);
        CAST_READ_JUMP(m_Header.ImportantColorsUsed, uint32_t, m_ImageBuff, jump);
        return;
    }
}

// -----------------------------------------------------------------------------
void Bitmap::MapImage()
{
    // Every row the countDown should be reseted to countDownNullVal
    const uint8_t countDownNullVal = -1;
    uint8_t countDown = countDownNullVal;
    
    // https://en.wikipedia.org/wiki/BMP_file_format#Pixel_storage

    // 'Possible loss of data' is my second name
#pragma warning ( push )
#pragma warning ( disable : 4244 )
    const uint64_t calcWidth = std::floorl((((m_Header.ColorDepth * m_Header.Width) + 31) / 32)) * 4;
#pragma warning ( pop )

    uint64_t row = 0;
    for (uint64_t i = m_Header.FileBeginOffset; 
        i < m_uSizeOfBuff; 
        i++, row++, countDown++)
    {
        if (row % calcWidth == 0)
        {
            row = 0;
            m_MappedImage.PushBackRow();
            countDown = countDownNullVal;
        }

        if (countDown >= 3)
        {
            m_MappedImage.PushBackPixel();
            countDown = 0;
        }
        switch (countDown)
        {
        case 0:
            m_MappedImage.LastPixel().SetBlueRef((uint8_t&)m_ImageBuff[i]);
            break;

        case 1:
            m_MappedImage.LastPixel().SetGreenRef((uint8_t&)m_ImageBuff[i]);
            break;

        case 2:
            m_MappedImage.LastPixel().SetRedRef((uint8_t&)m_ImageBuff[i]);
            break;

        default:
            throw;
        }
    }
}

#define CAST_WRITE_JUMP(loadFrom, dataType, buffer, jumpVal)   \
*((dataType*)&buffer[jumpVal]) = loadFrom;                     \
jumpVal += sizeof(dataType);

// -----------------------------------------------------------------------------
void SWBitmaps::Bitmap::MakeHeader()
{
    m_ImageBuff[0] = 'B';
    m_ImageBuff[1] = 'M';

    *(uint32_t*)(&m_ImageBuff[2]) = m_Header.FileSize;
    *(uint32_t*)(&m_ImageBuff[6]) = 0;
    *(uint32_t*)(&m_ImageBuff[10]) = m_Header.FileBeginOffset;

    if (m_Header.FileBeginOffset == BITMAPINFOHEADER)
    {
        uint8_t jump = 14;
        CAST_WRITE_JUMP(m_Header.SizeOfHeader, uint32_t, m_ImageBuff, jump);
        CAST_WRITE_JUMP(m_Header.Width, int32_t, m_ImageBuff, jump);
        CAST_WRITE_JUMP(m_Header.Height, int32_t, m_ImageBuff, jump);
        CAST_WRITE_JUMP(m_Header.ColorPlanes, uint16_t, m_ImageBuff, jump);
        CAST_WRITE_JUMP(m_Header.ColorDepth, uint16_t, m_ImageBuff, jump);
        CAST_WRITE_JUMP(m_Header.CompressionMethod, uint32_t, m_ImageBuff, jump);
        CAST_WRITE_JUMP(m_Header.ImageSize, uint32_t, m_ImageBuff, jump);
        CAST_WRITE_JUMP(m_Header.HorizontalResolution, int32_t, m_ImageBuff, jump);
        CAST_WRITE_JUMP(m_Header.VerticalResolution, int32_t, m_ImageBuff, jump);
        CAST_WRITE_JUMP(m_Header.ColorsInPalete, uint32_t, m_ImageBuff, jump);
        CAST_WRITE_JUMP(m_Header.ImportantColorsUsed, uint32_t, m_ImageBuff, jump);
        return;
    }
}
