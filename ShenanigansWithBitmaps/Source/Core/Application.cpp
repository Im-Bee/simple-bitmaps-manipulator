#include "Pch.h"

#include "Application.hpp"
#include "HexEditor.hpp"

// -----------------------------------------------------------------------------
void Application::Initialize()
{
    std::wcout << L"Availble commands\n\
        - 'q' to quit\n\
        - 'load' / 'l' to load a .bmp file from path\n\
        - 'save' / 's' to save a .bmp file in output dir ('./Output/FILE.bmp')\n\
        - 'color' to color whole image\n\
        - 'lookat' to view image in hex editor\n\
        - 'gray' to make image gray scale\n\
        - 'prt' to print image to terminal\n\
        - 'negative' to make image negative\n";

    FindPathToItself();
    CreateSaveDir();
}

// -----------------------------------------------------------------------------
#define SWB_IS_BITMAP                                       \
if (!m_pLoadedBitmap.get() ||                               \
    !m_pLoadedBitmap->IsValid())                            \
{                                                           \
    std::cout << "There is no file" << std::endl;           \
    return;                                                 \
}

// -----------------------------------------------------------------------------
void Application::Update()
{
    std::wstring r;
    std::cout << "...:";
    std::wcin >> r;
   
    // Normalize
    std::for_each(r.begin(), r.end(), [](wchar_t& c) {
        c = std::tolower(c);
        });

    if (r == L"q")
    {
        m_bQuit = true;
        return;
    }
    if (r == L"load" ||
        r == L"l")
    {
        LoadFile();
        return;
    }
    if (r == L"save" ||
        r == L"s")
    {
        
        SWB_IS_BITMAP;
        SaveFile();
        return;
    }
    if (r == L"lookat")
    {
        SWB_IS_BITMAP;
        LookAtFile();
        return;
    }
    if (r == L"color")
    {
        SWB_IS_BITMAP;
        m_pLoadedBitmap->ColorWhole({ 250, 170, 15 });
        return;
    }
    if (r == L"negative")
    {
        SWB_IS_BITMAP;
        m_pLoadedBitmap->MakeItNegative();
        return;
    }
    if (r == L"gray")
    {
        SWB_IS_BITMAP;
        m_pLoadedBitmap->MakeItGrayScale();
        return;
    }
    if (r == L"rnbw")
    {
        SWB_IS_BITMAP;
        m_pLoadedBitmap->MakeItRainbow();
        return;
    }
    if (r == L"ds")
    {
        SWB_IS_BITMAP;
        m_pLoadedBitmap->DeleteShadows();
        return;
    }
    if (r == L"prt")
    {
        SWB_IS_BITMAP;
        std::wstring w;
        std::cout << "Width:";
        std::wcin >> w;
        std::wstring b;
        std::cout << "Clamp [y/n]:";
        std::wcin >> b;
        SWHexEditor::Session::PrintImgFromGrayScale(m_pLoadedBitmap, std::stoi(w), std::tolower(b[0]) == 'y' ? true : false);
        return;
    }
    if (r == L"scl")
    {
        SWB_IS_BITMAP;
        m_pLoadedBitmap->ScaleTo(90, 0);
        return;
    }


    std::wcout << L"Invalid command" << std::endl;
}

// -----------------------------------------------------------------------------
void Application::Destroy()
{
    if (m_pLoadedBitmap.get())
        m_pLoadedBitmap->Destroy();
}

// Private ---------------------------------------------------------------------

// -----------------------------------------------------------------------------
void Application::LoadFile()
{
    if (m_pLoadedBitmap.get())
        m_pLoadedBitmap->Destroy();
    else
        m_pLoadedBitmap = std::make_shared<SWBitmaps::Bitmap>();

    std::wstring p;
    std::cout << "Path:";
    std::wcin >> p;

    // Remove all of '"'
    while (p.find('"') != std::string::npos)
    {
        auto at = p.find_first_of('"');
        p.erase(at, at + 1);
    }

    m_pLoadedBitmap->Initialize(p);
}   

// -----------------------------------------------------------------------------
void Application::SaveFile()
{
    static int uBitmapIndexCounter = 1;

    m_pLoadedBitmap->SaveToFile(SAVE_DIR 
        + L"Output" 
        + std::to_wstring(uBitmapIndexCounter++) 
        + L".bmp");
}

// -----------------------------------------------------------------------------
void Application::LookAtFile()
{
    auto s = SWHexEditor::Session();

    s.SetBuffer(m_pLoadedBitmap);
    s.Start();
    
    while (s.IsSessionAlive())
    {
        s.UpdateSession();
    }

    // Super secret windows exclusive feature
#ifdef _WIN32
    system("cls");
#endif // _WIN32

    std::wcout << L"Availble commands\n\
        - [q] for quit\n\
        - [load] to load a file from path\n\
        - [save] to save file in output dir\n\
        - [colorhalf] to color half\n\
        - [color] to color whole\n\
        - [lookat] to view file as dec \n\
        - [negative] to make file negative\n";
}

// -----------------------------------------------------------------------------
void Application::FindPathToItself()
{
#ifdef _WIN32
    wchar_t* tmpFileName = (wchar_t*) malloc(sizeof(wchar_t) * MAX_PATH);
    if (!tmpFileName)
        throw std::bad_alloc();

    GetModuleFileName(NULL, tmpFileName, MAX_PATH);

    wchar_t* lastSlash = wcsrchr(tmpFileName, L'\\');
    *(lastSlash + 1) = L'\0';

    m_PathToItself = tmpFileName;
    free(tmpFileName);

    return;
#endif // _WIN32
    throw;
}

// -----------------------------------------------------------------------------
void Application::CreateSaveDir()
{
#ifdef _WIN32
    CreateDirectory(SAVE_DIR.c_str(), NULL);

    return;
#endif // _WIN32

    throw;
}
