#pragma once

#include "Core/Bitmap.hpp"

#define SAVE_DIR (m_PathToItself.substr(0, m_PathToItself.size() - 1) + L"\\Output\\")

class Application
{
public:

    Application() = default;

    ~Application() = default;

public:

    void Initialize();

    void Update();

    void Destroy();

public:

    // Getters ---------------------------------------------------------------------

    const bool& GetQuit() const { return m_bQuit; }

private:

    void LoadFile();

    void SaveFile();

    void LookAtFile();

private:

    void FindPathToItself();

    void CreateSaveDir();

private:

    bool m_bQuit = false;

    std::wstring m_PathToItself = L"";
    
    std::shared_ptr<SWBitmaps::Bitmap> m_pLoadedBitmap = std::shared_ptr<SWBitmaps::Bitmap>(nullptr);

};
