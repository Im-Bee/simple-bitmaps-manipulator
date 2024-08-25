#include "Pch.h"

#include "HexEditor.hpp"
#include "Bitmap.hpp"

// -----------------------------------------------------------------------------
void SWHexEditor::Session::Start()
{
    StartUserControls();
}

// -----------------------------------------------------------------------------
void SWHexEditor::Session::UpdateSession()
{
    DrawOutput();
}

// -----------------------------------------------------------------------------
void SWHexEditor::Session::Stop()
{
    StopUserControls();
}

// -----------------------------------------------------------------------------
void SWHexEditor::Session::PrintImgFromGrayScale()
{
    if (!m_pTargetBitmap.get())
    {
        throw;
    }

    PrintImgFromGrayScale(m_pTargetBitmap, 90, true);
}

// ----------------------------------------------------------------------------
void SWHexEditor::Session::PrintImgFromGrayScale(IN std::shared_ptr<SWBitmaps::Bitmap> target, const uint8_t& width, const bool& clamp)
{
    std::vector<uint8_t> uPixelsForConsole;

    // Scale down the image -----------

    const uint8_t uNewWidth = width;
    // New height scaled to original aspect ratio
    const uint8_t uNewHeight = static_cast<uint8_t>(uNewWidth * ((long double)target->m_Header.Height / target->m_Header.Width));

    // Pre calculate
    uPixelsForConsole.resize((size_t)uNewHeight * uNewWidth);
    const long double fHeightRatio = (long double)target->m_Header.Height / uNewHeight;
    const long double fWidthRatio = (long double)target->m_Header.Width / uNewWidth;
 
    // Global index for tracking std::vector uPixelsForConsole
    uint64_t uGlobalIndex = 0;
    // Goes from top to bottom, 
    // because we print in console
    // from top to bottom
    for (int64_t i = uNewHeight - 1; i >= 0; i--)
    {
        for (int64_t k = 0; k < uNewWidth; k++)
        {
            auto p = target->m_MappedImage.Pixel(static_cast<size_t>(i * fHeightRatio),
                static_cast<size_t>(k * fWidthRatio));

            uPixelsForConsole[uGlobalIndex++] = ((uint32_t)p.Red() + p.Blue() + p.Green()) / 3;
        }
    }

    // Print out ----------------------

    const std::string colors = " .:-=o%@$";
    long double fMin, fMax;
    const uint8_t uIndexSizeOfColors = static_cast<uint8_t>(colors.size() - 1);
    if (clamp)
    {
        fMin = *std::min_element(uPixelsForConsole.begin(), uPixelsForConsole.end());
        fMax = *std::max_element(uPixelsForConsole.begin(), uPixelsForConsole.end());
    }
    else
    {
        fMin = 0;
        fMax = 255;
    }
    // Reused global index for tracking width of the image
    // Starting from 1 to not add additional '\n'
    uGlobalIndex = 1;
    for (auto& c : uPixelsForConsole)
    {
        uint8_t power = static_cast<uint8_t>(((c - fMin) / (fMax - fMin)) * uIndexSizeOfColors);
        std::cout << colors[power] << " ";
        
        if (uGlobalIndex % uNewWidth == 0)
            std::cout << '\n';
        
        uGlobalIndex++;
    }
}

// Setters ---------------------------------------------------------------------

// -----------------------------------------------------------------------------
void SWHexEditor::Session::SetBuffer(IN std::shared_ptr<SWBitmaps::Bitmap> target)
{
    if (m_UserControlThreadSwitch.load())
        return;
    
    m_pTargetBitmap = target;

    m_pTargetBuffer = target->GetRawPtr();
    m_uTargetBufferSize = target->GetRawSize();
}

// Private ---------------------------------------------------------------------

// -----------------------------------------------------------------------------
void SWHexEditor::Session::StartUserControls()
{
    if (m_UserControlThreadSwitch.load())
        StopUserControls();

    m_UserControlThreadSwitch.store(true);
    m_UserControlThread = std::thread(&SWHexEditor::Session::UserControlLoop, 
        this);
}

// -----------------------------------------------------------------------------
void SWHexEditor::Session::StopUserControls()
{
    m_UserControlThreadSwitch.store(false);
    
    if (m_UserControlThread.joinable())
        m_UserControlThread.join();
}

// -----------------------------------------------------------------------------
void SWHexEditor::Session::UserControlLoop()
{
    DWORD numberOfEvents;
    INPUT_RECORD iRec;
    HANDLE console = GetStdHandle(STD_INPUT_HANDLE);

    // Console not found
    if (console == NULL)
        throw;

    while (m_UserControlThreadSwitch.load())
    {
        ReadConsoleInput(console, &iRec, 1, &numberOfEvents);

        if (iRec.EventType != KEY_EVENT
            || !((KEY_EVENT_RECORD&)iRec.Event).bKeyDown)
        {
            continue;
        }

        if (((KEY_EVENT_RECORD&)iRec.Event).uChar.AsciiChar == 'q')
        {
            m_UserControlThreadSwitch.store(false);
            return;
        }

        if (((KEY_EVENT_RECORD&)iRec.Event).uChar.AsciiChar == 'w')
        {
            IncreaseHeight();
        }        
        if (((KEY_EVENT_RECORD&)iRec.Event).uChar.AsciiChar == 's')
        {
            DecreaseHeight();
        }
        if (((KEY_EVENT_RECORD&)iRec.Event).uChar.AsciiChar == 'd')
        {
            GoRight();
        }
        if (((KEY_EVENT_RECORD&)iRec.Event).uChar.AsciiChar == 'a')
        {
            GoLeft();
        }
        if (((KEY_EVENT_RECORD&)iRec.Event).uChar.AsciiChar == 'k')
        {
            IncreaseValue();
        }
        if (((KEY_EVENT_RECORD&)iRec.Event).uChar.AsciiChar == 'j')
        {
            DecreaseValue();
        }
        if (((KEY_EVENT_RECORD&)iRec.Event).uChar.AsciiChar == 'o')
        {
            if (m_DisplayMode == Hex)
                m_DisplayMode = Dec;
            else
                m_DisplayMode = Hex;
        }
    }
}

// -----------------------------------------------------------------------------
void SWHexEditor::Session::ClearScreen()
{
#ifdef _WIN32
    system("cls");

    return;
#endif // _WIN32

    throw;
}

// -----------------------------------------------------------------------------
std::string SWHexEditor::Session::PrintBufferRow(IN const uint64_t& i)
{
    const uint64_t startingIndex = (i * m_uRowWidth);

    // Outside of buffer scope
    if (i < 0 || startingIndex >= m_uTargetBufferSize)
        return "";

    bool isSelected;
    if (i == m_uHeightIndx)
        isSelected = true;
    else
        isSelected = false;
    
    std::string result;
    result += "Index ";
    result += std::format("{:6d}", i);
    result += " | ";
    if (isSelected)
        result += "---  ";
    else
        result += "    ";

    std::string tmp;
    for (uint64_t i = startingIndex; i < startingIndex + m_uRowWidth; i++)
    {
        if (isSelected &&
            i == startingIndex + m_uWidthIndx)
            tmp = " >";
        else
            tmp = " ";

        if (m_DisplayMode == Hex)
            tmp += std::format("{:2x}", (uint8_t)m_pTargetBuffer[i]);
        else if (m_DisplayMode == Dec)
            tmp += std::format("{:3d}", (uint8_t)m_pTargetBuffer[i]);

        if (isSelected && 
            i == startingIndex + m_uWidthIndx)
            tmp += "< ";
        else
            tmp += " ";

        result += tmp;
    }

    std::for_each(result.begin(), result.end(), [](char& c) {
        c = std::toupper(c);
        });

    if (isSelected)
        result += "---";
    else
    {
        result += "    ";
        result += "  ";
    }

    result += " | ";
    result += "\n";

    return result;
}

// -----------------------------------------------------------------------------
void SWHexEditor::Session::DrawOutput()
{
    using namespace std::chrono_literals;

    std::string output = {};
    const uint8_t offsetUpAndDown = 14;
    uint64_t upIndex;
    uint64_t downIndex = (m_uHeightIndx + offsetUpAndDown);

    // Hacky way around
    if (m_uHeightIndx < offsetUpAndDown)
    {
        upIndex = 0;
        downIndex += offsetUpAndDown - m_uHeightIndx;
    }
    else 
    {
        upIndex = m_uHeightIndx - offsetUpAndDown;
    }

    for (uint64_t& i = upIndex;
        i < downIndex;
        i++)
    { 
        output += PrintBufferRow(i);
    }
    
    std::cout << output;
    std::cout << std::endl << SWBytesManipulation_FOOTER;
    std::this_thread::sleep_for(11ms);
    ClearScreen();
}

// -----------------------------------------------------------------------------
void SWHexEditor::Session::IncreaseHeight()
{
    if (m_uHeightIndx == 0)
    {
        m_uHeightIndx = m_uTargetBufferSize / m_uRowWidth;
        return;
    }

    m_uHeightIndx--;
}

// -----------------------------------------------------------------------------
void SWHexEditor::Session::DecreaseHeight()
{
    if ((m_uHeightIndx + 1) * m_uRowWidth >= m_uTargetBufferSize)
    {
        m_uHeightIndx = 0;
        return;
    }

    m_uHeightIndx++;
}

// -----------------------------------------------------------------------------
void SWHexEditor::Session::GoRight()
{
    if ((m_uWidthIndx + 1) >= m_uRowWidth)
    {
        m_uWidthIndx = 0;
        return;
    }

    m_uWidthIndx++;
}

// -----------------------------------------------------------------------------
void SWHexEditor::Session::GoLeft()
{
    if (m_uWidthIndx == 0)
    {
        m_uWidthIndx = m_uRowWidth - 1;
        return;
    }

    m_uWidthIndx--;
}

// -----------------------------------------------------------------------------
void SWHexEditor::Session::IncreaseValue()
{
    m_pTargetBuffer[(m_uHeightIndx * m_uRowWidth) + m_uWidthIndx]++;
}

// -----------------------------------------------------------------------------
void SWHexEditor::Session::DecreaseValue()
{
    m_pTargetBuffer[(m_uHeightIndx * m_uRowWidth) + m_uWidthIndx]--;
}
