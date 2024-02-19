
#include "cef3lib.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#if PLATFORM_WINDOWS
#include "Windows/WindowsPlatformProcess.h"
#elif PLATFORM_LINUX
#include "Linux/LinuxPlatformProcess.h"
#elif PLATFORM_MAC
#include "Mac/MacPlatformProcess.h"
#endif
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include <string>
#include <stdlib.h>
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
#if PLATFORM_MAC
#	include "include/wrapper/cef_library_loader.h"
#endif
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

#if PLATFORM_MAC
	CefScopedLibraryLoader CEFLibraryLoader;
#endif
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
#elif PLATFORM_MAC
	LibPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/cefForUe"), TEXT(CEF3_VERSION), TEXT("mac/lib"));
#elif PLATFORM_LINUX
	LibPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/cefForUe"), TEXT(CEF3_VERSION), TEXT("linux/lib"));
#endif
	return LibPath;
}

void CEF3LIB::LoadCEF3Modules()
{
	if (dllHand.size())return;// has load
	//UE_LOG(CoreWebLog, Error, TEXT("CEF3DLL::LoadCEF3Modules"));
	FString libPath = LibPath();
#if PLATFORM_WINDOWS
	FPlatformProcess::PushDllDirectory(*libPath);
	if (LoadDllCEF(FPaths::Combine(*libPath, TEXT("chrome_elf.dll")))) {
		LoadDllCEF(FPaths::Combine(*libPath, TEXT("libcef.dll")));
		//LoadDllCEF(FPaths::Combine(*libPath, TEXT("d3dcompiler_47.dll")));
		//LoadDllCEF(FPaths::Combine(*libPath, TEXT("libGLESv2.dll")));
		//LoadDllCEF(FPaths::Combine(*libPath, TEXT("libEGL.dll")));
	}
	FPlatformProcess::PopDllDirectory(*libPath);
#elif PLATFORM_MAC
	FString frameWorks = FPaths::Combine(*libPath, TEXT("Chromium Embedded Framework.framework"), TEXT("Chromium Embedded Framework"));
	if (!cef_load_library(TCHAR_TO_ANSI(*frameWorks))) {
		UE_LOG(LogTemp, Error, TEXT("Chromium loader initialization failed"));
	}
#elif PLATFORM_LINUX
	FPlatformProcess::PushDllDirectory(*libPath);
	LoadDllCEF(FPaths::Combine(*libPath, TEXT("libcef.so")));
	FPlatformProcess::PopDllDirectory(*libPath);
#endif
}

void CEF3LIB::UnloadCEF3Modules()
{
	for (auto it = dllHand.rbegin(); it != dllHand.rend(); it++) {
		FPlatformProcess::FreeDllHandle(*it);
	}
	dllHand.clear();
}


