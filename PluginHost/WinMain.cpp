// WinMain.cpp

#pragma execution_character_set("utf-8") 

//---------------------------------------------------------------------------//
//
// アプリケーション エントリポイント
//   Copyright (C) 2014 tapetums
//
//---------------------------------------------------------------------------//

#include <windows.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "Application.hpp"
#include "UWnd.hpp"
#include "PluginCollection.hpp"

//---------------------------------------------------------------------------//

// メモリリーク検出
#if defined(_DEBUG) || (DEBUG)
    #define _CRTDBG_MAP_ALLOC
    #include <crtdbg.h>
    #define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

//---------------------------------------------------------------------------//

PluginCollection plugins;

//---------------------------------------------------------------------------//

int WINAPI wWinMain
(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR    lpCmdLine,
    INT32     nCmdShow
)
{
    int ret = 0;

    // メモリリーク検出
    #if defined(_DEBUG) || (DEBUG)
    {
        ::_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
    }
    #endif

    // COM の初期化
    ::CoInitialize(nullptr);

    static const auto listener = [](U8CSTR msg, IData* data) -> HRESULT
    {
        ::MessageBoxA(nullptr, "", (char*)msg, MB_OK);
        return S_OK;
    };

    // プラグインの読み込み
    {
        WCHAR dir_path[MAX_PATH];
        ::GetModuleFileNameW(nullptr, dir_path, MAX_PATH);
        ::PathRemoveFileSpecW(dir_path);
        SearchPlugins(dir_path, plugins, true);

        const auto size = plugins.size();
        for ( size_t index = 0; index < size; ++index )
        {
            const auto plugin = plugins.at(index);
            if ( nullptr == plugin )
            {
                break;
            }
            plugin->Attach((U8CSTR)"NotifyTest", listener);
            plugin->Attach((U8CSTR)"NotifyTest", listener);
            plugin->Notify((U8CSTR)"NotifyTest", nullptr);
            plugin->Detach((U8CSTR)"NotifyTest", listener);
            plugin->Detach((U8CSTR)"NotifyTest", listener);
        }
    }

    // プログラム本体の実行
    {
        UWnd wnd(TEXT("Test"));
        wnd.Create();
        wnd.Resize(400, 300);
        wnd.ToCenter();
        wnd.Show();

        // メインループ
        Application app;
        ret = app.Run();
    }

    // COM の終了処理
    ::CoUninitialize();

    return ret;
}

//---------------------------------------------------------------------------//

// WinMain.cpp