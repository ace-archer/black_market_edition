#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
//#include <windows.h>

#include <wtypes.h>
#include <stdio.h>

#include <system_error>
#include <Shlwapi.h>

#define ERROR_MESSAGE_CAPTION "BME Launcher EXE error"

HMODULE hLauncherModule;
HMODULE hHookModule;
FARPROC Launcher_LauncherMain;

FARPROC GetLauncherMain()
{
    static FARPROC Launcher_LauncherMain;
    if (!Launcher_LauncherMain)
        Launcher_LauncherMain = GetProcAddress(hLauncherModule, "LauncherMain");
    return Launcher_LauncherMain;
}

void LibraryLoadError(DWORD dwMessageId, const wchar_t* libName, const wchar_t* location)
{
    char text[2048];
    std::string message = std::system_category().message(dwMessageId);
    sprintf_s(text, "Failed to load the %ls at \"%ls\" (%lu):\n\n%hs", libName, location, dwMessageId, message.c_str());
    MessageBoxA(GetForegroundWindow(), text, ERROR_MESSAGE_CAPTION, 0);
}

bool GetExePathWide(wchar_t* dest, DWORD destSize)
{
    if (!dest) return NULL;
    if (destSize < MAX_PATH) return NULL;

    DWORD length = GetModuleFileNameW(NULL, dest, destSize);
    return length && PathRemoveFileSpecW(dest);
}

bool IsAnyIMEInstalled()
{
    auto count = GetKeyboardLayoutList(0, nullptr);
    if (count == 0)
        return false;
    auto* list = reinterpret_cast<HKL*>(_alloca(count * sizeof(HKL)));
    GetKeyboardLayoutList(count, list);
    for (int i = 0; i < count; i++)
        if (ImmIsIME(list[i]))
            return true;
    return false;
}

void SetMitigationPolicies()
{
    auto SetProcessMitigationPolicy = (decltype(&::SetProcessMitigationPolicy))GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "SetProcessMitigationPolicy");
    if (SetProcessMitigationPolicy)
    {
        PROCESS_MITIGATION_ASLR_POLICY ap;
        ap.EnableBottomUpRandomization = true;
        ap.EnableForceRelocateImages = true;
        ap.EnableHighEntropy = true;
        ap.DisallowStrippedImages = true; // Images that have not been built with /DYNAMICBASE and do not have relocation information will fail to load if this flag and EnableForceRelocateImages are set.
        SetProcessMitigationPolicy(ProcessASLRPolicy, &ap, sizeof(ap));

        /*PROCESS_MITIGATION_DYNAMIC_CODE_POLICY dcp;
        dcp.ProhibitDynamicCode = true;
        SetProcessMitigationPolicy(ProcessDynamicCodePolicy, &dcp, sizeof(dcp));*/

        if (!IsAnyIMEInstalled()) // this breaks IME apparently (https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-updateprocthreadattribute)
        {
            PROCESS_MITIGATION_EXTENSION_POINT_DISABLE_POLICY epdp;
            epdp.DisableExtensionPoints = true;
            SetProcessMitigationPolicy(ProcessExtensionPointDisablePolicy, &epdp, sizeof(epdp));
        }

        PROCESS_MITIGATION_CONTROL_FLOW_GUARD_POLICY cfgp;
        cfgp.EnableControlFlowGuard = true;
        //cfgp.StrictMode = true; // this needs to be disabled to load stubs with no CRT
        SetProcessMitigationPolicy(ProcessControlFlowGuardPolicy, &cfgp, sizeof(cfgp));

        PROCESS_MITIGATION_DEP_POLICY dp;
        dp.Enable = true;
        dp.Permanent = true;
        SetProcessMitigationPolicy(ProcessDEPPolicy, &dp, sizeof(dp));

        PROCESS_MITIGATION_IMAGE_LOAD_POLICY ilp;
        ilp.PreferSystem32Images = true;
        ilp.NoRemoteImages = true;
        SetProcessMitigationPolicy(ProcessImageLoadPolicy, &ilp, sizeof(ilp));

        PROCESS_MITIGATION_USER_SHADOW_STACK_POLICY usspp;
        usspp.EnableUserShadowStack = true;
        usspp.EnableUserShadowStackStrictMode = true;
        SetProcessMitigationPolicy(ProcessUserShadowStackPolicy, &usspp, sizeof(usspp));
    }
}

bool Load(LPSTR lpCmdLine)
{
    SetMitigationPolicies();

    wchar_t exePath[2048];
    wchar_t LibFullPath[2048];

    if (!GetExePathWide(exePath, 2048))
    {
        MessageBoxA(GetForegroundWindow(), "Failed getting game directory.\nThe game cannot continue and has to exit.", ERROR_MESSAGE_CAPTION, 0);
        return false;
    }

    if (_wchdir(exePath))
    {
        MessageBoxA(GetForegroundWindow(), "Failed changing current directory to game directory.\nThe game cannot continue and has to exit.", ERROR_MESSAGE_CAPTION, 0);
        return false;
    }

#define LOAD_LIBRARY(libname) swprintf_s(LibFullPath, L"%s\\" libname, exePath); if (!LoadLibraryExW(LibFullPath, 0, LOAD_WITH_ALTERED_SEARCH_PATH)) { LibraryLoadError(GetLastError(), L ## libname, LibFullPath); return false; }

    LOAD_LIBRARY("bin\\x64_retail\\tier0.dll");

    {
        swprintf_s(LibFullPath, L"%s\\bin\\x64_retail\\launcher.org.dll", exePath);
        hLauncherModule = LoadLibraryExW(LibFullPath, 0, LOAD_WITH_ALTERED_SEARCH_PATH);
        if (!hLauncherModule)
        {
            // fall back to regular launcher.dll
            swprintf_s(LibFullPath, L"%s\\bin\\x64_retail\\launcher.dll", exePath);
            hLauncherModule = LoadLibraryExW(LibFullPath, 0, LOAD_WITH_ALTERED_SEARCH_PATH);
        }
        if (hLauncherModule) Launcher_LauncherMain = GetProcAddress(hLauncherModule, "LauncherMain");
        if (!hLauncherModule || Launcher_LauncherMain == nullptr)
        {
            LibraryLoadError(GetLastError(), L"launcher.dll", LibFullPath);
            return false;
        }
    }

    LOAD_LIBRARY("bin\\x64_retail\\engine.dll");
    LOAD_LIBRARY("r1\\bin\\x64_retail\\client.dll");
    LOAD_LIBRARY("bin\\x64_retail\\filesystem_stdio.dll");
    LOAD_LIBRARY("bin\\x64_retail\\materialsystem_dx11.dll");
    LOAD_LIBRARY("bin\\x64_retail\\vstdlib.dll");
    LOAD_LIBRARY("bin\\x64_retail\\vguimatsurface.dll");
    LOAD_LIBRARY("bin\\x64_retail\\inputsystem.dll");

    if (!strstr(lpCmdLine, "-nobme"))
    {
        FARPROC Hook_Init = nullptr;
        {
            swprintf_s(LibFullPath, L"%s\\bme\\bme.dll", exePath);
            hHookModule = LoadLibraryExW(LibFullPath, 0, LOAD_WITH_ALTERED_SEARCH_PATH);
            if (hHookModule) Hook_Init = GetProcAddress(hHookModule, "Init");
            if (!hHookModule || Hook_Init == nullptr)
            {
                LibraryLoadError(GetLastError(), L"bme.dll", LibFullPath);
                return false;
            }
        }

        //printf("before hook init\n");
        ((void (*)()) Hook_Init)();
    }
    return true;
}

//extern "C" _declspec(dllexport) void LauncherMain()
//extern "C" __declspec(dllexport) int LauncherMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
int __stdcall WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
    if (!Load(lpCmdLine))
        return 1;

    //auto LauncherMain = GetLauncherMain();
    //auto result = ((__int64(__fastcall*)())LauncherMain)();
    //auto result = ((signed __int64(__fastcall*)(__int64))LauncherMain)(0i64);
    return ((int(*)(HINSTANCE, HINSTANCE, LPSTR, int))Launcher_LauncherMain)(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
}

extern "C"
{
    __declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001;
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
