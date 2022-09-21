
#include "cef3lib.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#if PLATFORM_WINDOWS
#include "Windows/WindowsPlatformProcess.h"
#elif PLATFORM_LINUX
#include "Linux/LinuxPlatformProcess.h"
#endif
#include "Misc/Paths.h"
// WEB_CORE_API

//
//#if PLATFORM_WINDOWS
//#define CEFLIB_EXPORT __declspec(dllimport)
//#elif PLATFORM_LINUX
//#define CEFLIB_EXPORT 
//#endif
//#ifdef __cplusplus
//extern "C" {
//#endif
//	CEFLIB_EXPORT const char* cef_api_hash(int entry);
//#ifdef __cplusplus
//}
//#endif
//
//void CefEnableHighDPISupport();

// WEB_CORE_API
class CEF3LIB: public ICEF3LIB {
public:
	void LoadCEF3Modules() ;
	void UnloadCEF3Modules() ;
	FString LibPath() ;
	virtual ~CEF3LIB() = default;
private:
	void* LoadDllCEF(const FString& Path);
private:
	std::vector<void*> dllHand;
};

ICEF3LIB* ICEF3LIB::get() {
	static ICEF3LIB* install= nullptr;
	//UE_LOG(CoreWebLog, Error, TEXT("CEF3DLL::get"));
	if (nullptr == install) {
		install = new CEF3LIB();
	}
	return install;
}

void* CEF3LIB::LoadDllCEF(const FString& Path)
{
	if (Path.IsEmpty())
	{
		return nullptr;
	}
	void* Handle = FPlatformProcess::GetDllHandle(*Path);
	if (!Handle)
	{
		int32 ErrorNum = FPlatformMisc::GetLastError();
		TCHAR ErrorMsg[1024];
		FPlatformMisc::GetSystemErrorMessage(ErrorMsg, 1024, ErrorNum);
		UE_LOG(LogTemp, Error, TEXT("Failed to get CEF3 DLL handle for %s: %s (%d)"), *Path, ErrorMsg, ErrorNum);
	}
	else {
		dllHand.push_back(Handle);
	}
	return Handle;
}

FString CEF3LIB::LibPath() {
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("CefBase"));
	if (!Plugin.IsValid()) {
		Plugin = IPluginManager::Get().FindPlugin(TEXT("WebView"));
	}
	const FString BaseDir = FPaths::ConvertRelativePathToFull(Plugin->GetBaseDir());
	FString LibPath;
#if PLATFORM_WINDOWS
	LibPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/cefForUe"), TEXT(CEF3_VERSION), TEXT("win64/lib"));
#elif PLATFORM_LINUX
	LibPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/cefForUe"), TEXT("cef_103.5060"), TEXT("linux/lib"));
#endif
	return LibPath;
}

void CEF3LIB::LoadCEF3Modules()
{
	//UE_LOG(CoreWebLog, Error, TEXT("CEF3DLL::LoadCEF3Modules"));
	FString libPath = LibPath();
#if PLATFORM_WINDOWS
	FPlatformProcess::PushDllDirectory(*libPath);
	if (LoadDllCEF(FPaths::Combine(*libPath, TEXT("libcef.dll")))) {
		LoadDllCEF(FPaths::Combine(*libPath, TEXT("chrome_elf.dll")));
		LoadDllCEF(FPaths::Combine(*libPath, TEXT("d3dcompiler_47.dll")));
		LoadDllCEF(FPaths::Combine(*libPath, TEXT("libGLESv2.dll")));
		LoadDllCEF(FPaths::Combine(*libPath, TEXT("libEGL.dll")));
	}
	FPlatformProcess::PopDllDirectory(*libPath);
	//const char* api = cef_api_hash(0);
#elif PLATFORM_LINUX
	FPlatformProcess::PushDllDirectory(*libPath);
	LoadDllCEF(FPaths::Combine(*libPath, TEXT("libcef.so")));
	FPlatformProcess::PopDllDirectory(*libPath);
    FString cmd = FString::Printf(TEXT("chmod 775 %s"),*FPaths::Combine(*libPath, TEXT("cefhelper")));
	system(TCHAR_TO_UTF8(*cmd));
#endif
	//CefEnableHighDPISupport();
	//FString fAPI = api;
	//UE_LOG(LogTemp, Log, TEXT("API=[%s]"), *fAPI);
}

void CEF3LIB::UnloadCEF3Modules()
{
	for (auto it = dllHand.rbegin(); it != dllHand.rend(); it++) {
		FPlatformProcess::FreeDllHandle(*it);
	}
	dllHand.clear();
}


