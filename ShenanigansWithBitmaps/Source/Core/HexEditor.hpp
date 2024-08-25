#pragma once

#include "Bitmap.hpp"

#define SWBytesManipulation_FOOTER \
"W - up; S - down; A - left; D - right; j - decrease value; k - increase value; o - change between Hex and Dec mode"

namespace SWHexEditor
{
    enum DisplayMode
    {
        Hex,
        Dec
    };

    class Session
    {
    public:

        Session() = default;

        ~Session()
        {
            StopUserControls();
        }

    public:

        void Start();
        
        void UpdateSession();

        void Stop();

        void PrintImgFromGrayScale();

        static void PrintImgFromGrayScale(IN std::shared_ptr<SWBitmaps::Bitmap> target, const uint8_t& width, const bool& clamp);


    public:

        // Getters -------------------------------------------------------------

        bool IsSessionAlive() { return m_UserControlThreadSwitch.load(); }

    public:

        // Setters -------------------------------------------------------------
        
        void SetBuffer(IN std::shared_ptr<SWBitmaps::Bitmap> target);

    private:

        void StartUserControls();

        void StopUserControls();

        void UserControlLoop();

        void ClearScreen();

        std::string PrintBufferRow(IN const uint64_t& i);

        void DrawOutput();

    private:

        // User Controls -------------------------------------------------------

        void IncreaseHeight();

        void DecreaseHeight();

        void GoRight();

        void GoLeft();

        void IncreaseValue();

        void DecreaseValue();

    private:

        std::atomic_bool m_UserControlThreadSwitch = false;
        std::thread m_UserControlThread;

        std::shared_ptr<SWBitmaps::Bitmap> m_pTargetBitmap = std::shared_ptr<SWBitmaps::Bitmap>(nullptr);
        uint64_t m_uTargetBufferSize = 0;
        char* m_pTargetBuffer = nullptr;

        uint64_t m_uWidthIndx = 0;
        const uint64_t m_uRowWidth = 16;
        uint64_t m_uHeightIndx = 0;

        DisplayMode m_DisplayMode = Hex;

    };
}