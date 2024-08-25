#pragma once

#define BITMAP_CHUNK 4096

#pragma region Predeclarations

namespace SWHexEditor
{
    class Session;
}

#pragma endregion


namespace SWBitmaps
{
    struct Color
    {
        uint8_t Red = 0;
        uint8_t Green = 0;
        uint8_t Blue = 0;

        uint8_t& operator[](IN const uint8_t& i)
        {
            switch (i)
            {
            case 0:
                return Red;

            case 1:
                return Green;

            case 2:
                return Blue;

            default:
                throw;
            }
        }
    };

#pragma region Color definitions

    #define SWBITMAPS_COLOR_BLACK Color({0, 0, 0})
    #define SWBITMAPS_COLOR_WHITE Color({255, 255, 255})

#pragma endregion

    struct MappedPixel
    {
    public:

        MappedPixel() = default;

        MappedPixel(IN uint8_t* r, IN uint8_t* g, IN uint8_t* b) :
            m_pRed(r),
            m_pGreen(g),
            m_pBlue(b)
        {};

    public:

        // Getters -------------------------------------------------------------

        uint8_t& Red() { return *m_pRed; }

        uint8_t& Green() { return *m_pGreen; }

        uint8_t& Blue() { return *m_pBlue; }

    public:

        // Setters -------------------------------------------------------------

        void SetRedRef(IN uint8_t& r) { m_pRed = &r; }

        void SetGreenRef(IN uint8_t& g) { m_pGreen = &g; }

        void SetBlueRef(IN uint8_t& b) { m_pBlue = &b; }

    public:

        // Operators -----------------------------------------------------------

        static bool IsEmpty(IN const MappedPixel& mp)
        {
            if (mp.m_pRed == nullptr &&
                mp.m_pGreen == nullptr &&
                mp.m_pBlue == nullptr)
                return true;

            return false;
        }

        static bool IsInvalid(IN const MappedPixel& mp)
        {
            if (mp.m_pRed == nullptr ||
                mp.m_pGreen == nullptr ||
                mp.m_pBlue == nullptr)
                return true;

            return false;
        }

        bool operator==(IN const MappedPixel& right)
        {
            if (m_pRed == right.m_pRed &&
                m_pGreen == right.m_pGreen &&
                m_pBlue == right.m_pBlue)
                return true;

            return false;
        }

    private:

        uint8_t* m_pRed = nullptr;
        uint8_t* m_pGreen = nullptr;
        uint8_t* m_pBlue = nullptr;

    };

    // -----------------------------------------------------------------------------
    #define SW_THROW_IF_I_OUT_OF_SCOPE(i)   \
    if (i >= m_Map.size())                  \
        throw;

    struct PixelMapWrapper
    {
    public:

        PixelMapWrapper() = default;

        ~PixelMapWrapper() = default;

    public:

        void PushBackRow()
        {
            m_Map.push_back({});
        }

        void PushBackPixel()
        {
            if (m_Map.size() == 0)
                PushBackRow();

            m_Map.back().push_back(MappedPixel());
        }

        void PushBackPixel(IN const size_t& i)
        {
            SW_THROW_IF_I_OUT_OF_SCOPE(i);

            m_Map[i].push_back(MappedPixel());
        }

        void Clear()
        {
            m_Map.clear();
        }

    public:

        // Getters ---------------------------------------------------------------------

        std::vector<MappedPixel>& Row(IN const size_t& i)
        {
            SW_THROW_IF_I_OUT_OF_SCOPE(i);

            return m_Map[i];
        }

        MappedPixel& LastPixel()
        {
            return m_Map.back().back();
        }

        MappedPixel& Pixel(IN const size_t& row, IN const size_t& col)
        {
            SW_THROW_IF_I_OUT_OF_SCOPE(row);
            if (col >= m_Map[row].size())
                throw;

            return m_Map[row][col];
        }

        std::vector<MappedPixel>& operator[](IN const size_t& i)
        {
            SW_THROW_IF_I_OUT_OF_SCOPE(i);

            return m_Map[i];
        }

        size_t GetWidth(IN const size_t& i)
        {
            SW_THROW_IF_I_OUT_OF_SCOPE(i);

            return m_Map[i].size();
        }

        size_t GetHeight()
        {
            return m_Map.size();
        }

    private:

        std::vector<std::vector<MappedPixel>> m_Map;

    };
    
#pragma region Headers types 
    #define BITMAPINFOHEADER (14 + 40)
#pragma endregion

    struct BitmapHeader
    {
        bool Valid = false;
        uint32_t FileSize = 0;
        uint32_t FileBeginOffset = 0;
        uint32_t SizeOfHeader = 0;
        int32_t Width = 0;
        int32_t Height = 0;
        uint16_t ColorPlanes = 0;
        uint16_t ColorDepth = 0;
        uint32_t CompressionMethod = 0;
        uint32_t ImageSize = 0;
        int32_t HorizontalResolution = 0;
        int32_t VerticalResolution = 0;
        uint32_t ColorsInPalete = 0;
        uint32_t ImportantColorsUsed = 0;
    };

    class Bitmap
    {
        
        friend SWHexEditor::Session;

    public:

        Bitmap() = default;

        ~Bitmap()
        {
            Destroy();
        };

    public:

        void operator=(const Bitmap& b)
        {
            m_Header = b.m_Header;
            m_uSizeOfBuff = b.m_uSizeOfBuff;

            m_ImageBuff = (char*)malloc(sizeof(char) * m_uSizeOfBuff);
            _memccpy(m_ImageBuff, b.m_ImageBuff, sizeof(char), m_uSizeOfBuff);

            m_MappedImage = b.m_MappedImage;
        }

    public:

        void Initialize(IN const std::wstring& path);

        void Destroy();

    public:

        void SaveToFile(IN const std::wstring& path);

    public:

        // Image manipulation ----------------------------------------------------------

        void ScaleTo(uint32_t width, uint32_t height);

        void ColorWhole(IN Color c);

        void ColorHalf(IN Color c);

        void MakeItBlack()
        {
            ColorWhole(SWBITMAPS_COLOR_BLACK);
        }

        void MakeItWhite()
        {
            ColorWhole(SWBITMAPS_COLOR_WHITE);
        }

        void MakeItRainbow();

        void MakeItNegative();

        void MakeItGrayScale();

        void DeleteShadows();

    public:

        // Getters -------------------------------------------------------------

        const bool& IsValid() const { return m_Header.Valid; }

    private:

        // Private, for friend class -------------------------------------------

        char* GetRawPtr() { return m_ImageBuff; }

        const uint64_t& GetRawSize() { return m_uSizeOfBuff; }

    private:

        void LoadFromPath();

        void ReadHeader();

        void MapImage();

        void MakeHeader();

    private:

        std::wstring m_Path = L"";

        uint64_t m_uSizeOfBuff = 0;
        char* m_ImageBuff = nullptr;
        
        BitmapHeader m_Header = {};
        PixelMapWrapper m_MappedImage = {};
    };
}
