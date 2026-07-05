#include "native/module_api.h"
#include "defs/module_help.h"
#include <windows.h>

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>

#include "filesystem.h"
#include "tier1/interface.h"

#include "moduleapi/hooks.h"
#include "native/detour_manager.h"
#include "native/interface_registry.h"
#include "native/log.h"


using namespace gm_laboratory;

// enable this if you want to see every function call to the filesystem
// #define USE_FS_TRACE
#if USE_FS_TRACE
#define FSTRACE(name) Log("example3", "IFileSystem::%s\n", name)
#else
#define FSTRACE(name)
#endif

MODULE_START()

abstract_class IBaseFileSystemGmod{
public:
	virtual int Read(void* pOutput, int size, FileHandle_t file) = 0;
	virtual int Write(void const* pInput, int size, FileHandle_t file) = 0;
	virtual FileHandle_t Open(const char* pFileName, const char* pOptions, const char* pathID) = 0;
	virtual void Close(FileHandle_t file) = 0;
	virtual void Seek(FileHandle_t file, long long pos, FileSystemSeek_t seekType) = 0;
	virtual unsigned long long Tell(FileHandle_t file) = 0;
	virtual unsigned long long Size(FileHandle_t file) = 0;
	virtual unsigned long long Size(const char* pFileName, const char* pPathID) = 0;
	virtual void Flush(FileHandle_t file) = 0;
	virtual bool Precache(const char* pFileName, const char* pPathID) = 0;
	virtual bool FileExists(const char* pFileName, const char* pPathID) = 0;
	virtual bool IsFileWritable(char const* pFileName, const char* pPathID) = 0;
	virtual bool SetFileWritable(char const* pFileName, bool writable, const char* pPathID) = 0;
	virtual long GetFileTime(const char* pFileName, const char* pPathID) = 0;
	virtual bool ReadFile(const char* pFileName, const char* pPath, CUtlBuffer& buf, int nMaxBytes, int nStartingByte, FSAllocFunc_t pfnAlloc) = 0;
	virtual bool WriteFile(const char* pFileName, const char* pPath, CUtlBuffer& buf) = 0;
	virtual bool UnzipFile(const char* pFileName, const char* pPath, const char* pDestination) = 0;
};

class CLoggingFileSystem : public IAppSystem, public IBaseFileSystemGmod {
public:
	explicit CLoggingFileSystem(CLoggingFileSystem* real) : m_real(real) {}

	bool Connect(CreateInterfaceFn factory) override { FSTRACE("Connect"); return m_real->Connect(factory); }
	void Disconnect() override { FSTRACE("Disconnect"); m_real->Disconnect(); }
	void* QueryInterface(const char* pInterfaceName) override { FSTRACE("QueryInterface"); return m_real->QueryInterface(pInterfaceName); }
	InitReturnVal_t Init() override { FSTRACE("Init"); return m_real->Init(); }
	void Shutdown() override { FSTRACE("Shutdown"); m_real->Shutdown(); }
	const AppSystemInfo_t* GetDependencies() override { FSTRACE("GetDependencies"); return m_real->GetDependencies(); }
	AppSystemTier_t GetTier() override { FSTRACE("GetTier"); return m_real->GetTier(); }
	void Reconnect(CreateInterfaceFn factory, const char* pInterfaceName) override { FSTRACE("Reconnect"); m_real->Reconnect(factory, pInterfaceName); }
	bool IsSingleton() override { FSTRACE("IsSingleton"); return m_real->IsSingleton(); }

	int Read(void* pOutput, int size, FileHandle_t file) override { FSTRACE("Read"); return m_real->Read(pOutput, size, file); }
	int Write(void const* pInput, int size, FileHandle_t file) override { FSTRACE("Write"); return m_real->Write(pInput, size, file); }
	FileHandle_t Open(const char* pFileName, const char* pOptions, const char* pathID) override { FSTRACE("Open"); return m_real->Open(pFileName, pOptions, pathID); }
	void Close(FileHandle_t file) override { FSTRACE("Close"); m_real->Close(file); }
	void Seek(FileHandle_t file, long long pos, FileSystemSeek_t seekType) override { FSTRACE("Seek"); m_real->Seek(file, pos, seekType); }
	unsigned long long Tell(FileHandle_t file) override { FSTRACE("Tell"); return m_real->Tell(file); }
	unsigned long long Size(FileHandle_t file) override { FSTRACE("Size(FileHandle_t)"); return m_real->Size(file); }
	unsigned long long Size(const char* pFileName, const char* pPathID) override { FSTRACE("Size(const char*)"); return m_real->Size(pFileName, pPathID); }
	void Flush(FileHandle_t file) override { FSTRACE("Flush"); m_real->Flush(file); }
	bool Precache(const char* pFileName, const char* pPathID) override { FSTRACE("Precache"); return m_real->Precache(pFileName, pPathID); }
	bool FileExists(const char* pFileName, const char* pPathID) override { FSTRACE("FileExists"); return m_real->FileExists(pFileName, pPathID); }
	bool IsFileWritable(char const* pFileName, const char* pPathID) override { FSTRACE("IsFileWritable"); return m_real->IsFileWritable(pFileName, pPathID); }
	bool SetFileWritable(char const* pFileName, bool writable, const char* pPathID) override { FSTRACE("SetFileWritable"); return m_real->SetFileWritable(pFileName, writable, pPathID); }
	long GetFileTime(const char* pFileName, const char* pPathID) override { FSTRACE("GetFileTime"); return m_real->GetFileTime(pFileName, pPathID); }
	bool ReadFile(const char* pFileName, const char* pPath, CUtlBuffer& buf, int nMaxBytes, int nStartingByte, FSAllocFunc_t pfnAlloc) override { FSTRACE("ReadFile"); return m_real->ReadFile(pFileName, pPath, buf, nMaxBytes, nStartingByte, pfnAlloc); }
	bool WriteFile(const char* pFileName, const char* pPath, CUtlBuffer& buf) override { FSTRACE("WriteFile"); return m_real->WriteFile(pFileName, pPath, buf); }
	bool UnzipFile(const char* pFileName, const char* pPath, const char* pDestination) override { FSTRACE("UnzipFile"); return m_real->UnzipFile(pFileName, pPath, pDestination); }

	virtual bool IsSteam() const { FSTRACE("IsSteam"); return m_real->IsSteam(); }
	virtual FilesystemMountRetval_t MountSteamContent(int nExtraAppId) { FSTRACE("MountSteamContent"); return m_real->MountSteamContent(nExtraAppId); }

	virtual void AddSearchPath(const char* pPath, const char* pathID, SearchPathAdd_t addType) { FSTRACE("AddSearchPath"); m_real->AddSearchPath(pPath, pathID, addType); }
	virtual bool RemoveSearchPath(const char* pPath, const char* pathID) { FSTRACE("RemoveSearchPath"); return m_real->RemoveSearchPath(pPath, pathID); }
	virtual void RemoveAllSearchPaths() { FSTRACE("RemoveAllSearchPaths"); m_real->RemoveAllSearchPaths(); }
	virtual void RemoveSearchPaths(const char* szPathID) { FSTRACE("RemoveSearchPaths"); m_real->RemoveSearchPaths(szPathID); }
	virtual void MarkPathIDByRequestOnly(const char* pPathID, bool bRequestOnly) { FSTRACE("MarkPathIDByRequestOnly"); m_real->MarkPathIDByRequestOnly(pPathID, bRequestOnly); }
	virtual const char* RelativePathToFullPath(const char* pFileName, const char* pPathID, char* pDest, int maxLenInChars, PathTypeFilter_t pathFilter, PathTypeQuery_t* pPathType) { FSTRACE("RelativePathToFullPath"); return m_real->RelativePathToFullPath(pFileName, pPathID, pDest, maxLenInChars, pathFilter, pPathType); }
	virtual int GetSearchPath(const char* pathID, bool bGetPackFiles, char* pDest, int maxLenInChars) { FSTRACE("GetSearchPath"); return m_real->GetSearchPath(pathID, bGetPackFiles, pDest, maxLenInChars); }
	virtual bool AddPackFile(const char* fullpath, const char* pathID) { FSTRACE("AddPackFile"); return m_real->AddPackFile(fullpath, pathID); }

	virtual void RemoveFile(char const* pRelativePath, const char* pathID) { FSTRACE("RemoveFile"); m_real->RemoveFile(pRelativePath, pathID); }
	virtual bool RenameFile(char const* pOldPath, char const* pNewPath, const char* pathID) { FSTRACE("RenameFile"); return m_real->RenameFile(pOldPath, pNewPath, pathID); }
	virtual void CreateDirHierarchy(const char* path, const char* pathID) { FSTRACE("CreateDirHierarchy"); m_real->CreateDirHierarchy(path, pathID); }
	virtual bool IsDirectory(const char* pFileName, const char* pathID) { FSTRACE("IsDirectory"); return m_real->IsDirectory(pFileName, pathID); }
	virtual void FileTimeToString(char* pStrip, int maxCharsIncludingTerminator, long fileTime) { FSTRACE("FileTimeToString"); m_real->FileTimeToString(pStrip, maxCharsIncludingTerminator, fileTime); }

	virtual void SetBufferSize(FileHandle_t file, unsigned nBytes) { FSTRACE("SetBufferSize"); m_real->SetBufferSize(file, nBytes); }
	virtual bool IsOk(FileHandle_t file) { FSTRACE("IsOk"); return m_real->IsOk(file); }
	virtual bool EndOfFile(FileHandle_t file) { FSTRACE("EndOfFile"); return m_real->EndOfFile(file); }
	virtual char* ReadLine(char* pOutput, int maxChars, FileHandle_t file) { FSTRACE("ReadLine"); return m_real->ReadLine(pOutput, maxChars, file); }
	virtual int FPrintf(FileHandle_t file, const char* pFormat, ...) {
		FSTRACE("FPrintf");
		char str[8192];
		va_list marker;
		va_start(marker, pFormat);
		_vsnprintf(str, sizeof(str), pFormat, marker);
		va_end(marker);
		return m_real->FPrintf(file, "%s", str);
	}

	virtual CSysModule* LoadModule(const char* pFileName, const char* pPathID, bool bValidatedDllOnly) { FSTRACE("LoadModule"); return m_real->LoadModule(pFileName, pPathID, bValidatedDllOnly); }
	virtual void UnloadModule(CSysModule* pModule) { FSTRACE("UnloadModule"); m_real->UnloadModule(pModule); }

	virtual const char* FindFirst(const char* pWildCard, FileFindHandle_t* pHandle) { FSTRACE("FindFirst"); return m_real->FindFirst(pWildCard, pHandle); }
	virtual const char* FindNext(FileFindHandle_t handle) { FSTRACE("FindNext"); return m_real->FindNext(handle); }
	virtual bool FindIsDirectory(FileFindHandle_t handle) { FSTRACE("FindIsDirectory"); return m_real->FindIsDirectory(handle); }
	virtual void FindClose(FileFindHandle_t handle) { FSTRACE("FindClose"); m_real->FindClose(handle); }
	virtual const char* FindFirstEx(const char* pWildCard, const char* pPathID, FileFindHandle_t* pHandle) { FSTRACE("FindFirstEx"); return m_real->FindFirstEx(pWildCard, pPathID, pHandle); }

	virtual const char* GetLocalPath(const char* pFileName, char* pDest, int maxLenInChars) { FSTRACE("GetLocalPath"); return m_real->GetLocalPath(pFileName, pDest, maxLenInChars); }
	virtual bool FullPathToRelativePath(const char* pFullpath, char* pDest, int maxLenInChars) { FSTRACE("FullPathToRelativePath"); return m_real->FullPathToRelativePath(pFullpath, pDest, maxLenInChars); }
	virtual bool GetCurrentDirectory(char* pDirectory, int maxlen) { FSTRACE("GetCurrentDirectory"); return m_real->GetCurrentDirectory(pDirectory, maxlen); }

	virtual FileNameHandle_t FindOrAddFileName(char const* pFileName) { FSTRACE("FindOrAddFileName"); return m_real->FindOrAddFileName(pFileName); }
	virtual bool String(const FileNameHandle_t& handle, char* buf, int buflen) { FSTRACE("String"); return m_real->String(handle, buf, buflen); }

	virtual FSAsyncStatus_t AsyncReadMultiple(const FileAsyncRequest_t* pRequests, int nRequests, FSAsyncControl_t* phControls) { FSTRACE("AsyncReadMultiple"); return m_real->AsyncReadMultiple(pRequests, nRequests, phControls); }
	virtual FSAsyncStatus_t AsyncAppend(const char* pFileName, const void* pSrc, int nSrcBytes, bool bFreeMemory, FSAsyncControl_t* pControl) { FSTRACE("AsyncAppend"); return m_real->AsyncAppend(pFileName, pSrc, nSrcBytes, bFreeMemory, pControl); }
	virtual FSAsyncStatus_t AsyncAppendFile(const char* pAppendToFileName, const char* pAppendFromFileName, FSAsyncControl_t* pControl) { FSTRACE("AsyncAppendFile"); return m_real->AsyncAppendFile(pAppendToFileName, pAppendFromFileName, pControl); }
	virtual void AsyncFinishAll(int iToPriority) { FSTRACE("AsyncFinishAll"); m_real->AsyncFinishAll(iToPriority); }
	virtual void AsyncFinishAllWrites() { FSTRACE("AsyncFinishAllWrites"); m_real->AsyncFinishAllWrites(); }
	virtual FSAsyncStatus_t AsyncFlush() { FSTRACE("AsyncFlush"); return m_real->AsyncFlush(); }
	virtual bool AsyncSuspend() { FSTRACE("AsyncSuspend"); return m_real->AsyncSuspend(); }
	virtual bool AsyncResume() { FSTRACE("AsyncResume"); return m_real->AsyncResume(); }
	virtual void AsyncAddFetcher(IAsyncFileFetch* pFetcher) { FSTRACE("AsyncAddFetcher"); m_real->AsyncAddFetcher(pFetcher); }
	virtual void AsyncRemoveFetcher(IAsyncFileFetch* pFetcher) { FSTRACE("AsyncRemoveFetcher"); m_real->AsyncRemoveFetcher(pFetcher); }
	virtual FSAsyncStatus_t AsyncBeginRead(const char* pszFile, FSAsyncFile_t* phFile) { FSTRACE("AsyncBeginRead"); return m_real->AsyncBeginRead(pszFile, phFile); }
	virtual FSAsyncStatus_t AsyncEndRead(FSAsyncFile_t hFile) { FSTRACE("AsyncEndRead"); return m_real->AsyncEndRead(hFile); }
	virtual FSAsyncStatus_t AsyncFinish(FSAsyncControl_t hControl, bool wait) { FSTRACE("AsyncFinish"); return m_real->AsyncFinish(hControl, wait); }
	virtual FSAsyncStatus_t AsyncGetResult(FSAsyncControl_t hControl, void** ppData, int* pSize) { FSTRACE("AsyncGetResult"); return m_real->AsyncGetResult(hControl, ppData, pSize); }
	virtual FSAsyncStatus_t AsyncAbort(FSAsyncControl_t hControl) { FSTRACE("AsyncAbort"); return m_real->AsyncAbort(hControl); }
	virtual FSAsyncStatus_t AsyncStatus(FSAsyncControl_t hControl) { FSTRACE("AsyncStatus"); return m_real->AsyncStatus(hControl); }
	virtual FSAsyncStatus_t AsyncSetPriority(FSAsyncControl_t hControl, int newPriority) { FSTRACE("AsyncSetPriority"); return m_real->AsyncSetPriority(hControl, newPriority); }
	virtual void AsyncAddRef(FSAsyncControl_t hControl) { FSTRACE("AsyncAddRef"); m_real->AsyncAddRef(hControl); }
	virtual void AsyncRelease(FSAsyncControl_t hControl) { FSTRACE("AsyncRelease"); m_real->AsyncRelease(hControl); }

	virtual WaitForResourcesHandle_t WaitForResources(const char* resourcelist) { FSTRACE("WaitForResources"); return m_real->WaitForResources(resourcelist); }
	virtual bool GetWaitForResourcesProgress(WaitForResourcesHandle_t handle, float* progress, bool* complete) { FSTRACE("GetWaitForResourcesProgress"); return m_real->GetWaitForResourcesProgress(handle, progress, complete); }
	virtual void CancelWaitForResources(WaitForResourcesHandle_t handle) { FSTRACE("CancelWaitForResources"); m_real->CancelWaitForResources(handle); }
	virtual int HintResourceNeed(const char* hintlist, int forgetEverything) { FSTRACE("HintResourceNeed"); return m_real->HintResourceNeed(hintlist, forgetEverything); }
	virtual bool IsFileImmediatelyAvailable(const char* pFileName) { FSTRACE("IsFileImmediatelyAvailable"); return m_real->IsFileImmediatelyAvailable(pFileName); }
	virtual void GetLocalCopy(const char* pFileName) { FSTRACE("GetLocalCopy"); m_real->GetLocalCopy(pFileName); }

	virtual void PrintOpenedFiles() { FSTRACE("PrintOpenedFiles"); m_real->PrintOpenedFiles(); }
	virtual void PrintSearchPaths() { FSTRACE("PrintSearchPaths"); m_real->PrintSearchPaths(); }
	virtual void SetWarningFunc(void (*pfnWarning)(const char* fmt, ...)) { FSTRACE("SetWarningFunc"); m_real->SetWarningFunc(pfnWarning); }
	virtual void SetWarningLevel(FileWarningLevel_t level) { FSTRACE("SetWarningLevel"); m_real->SetWarningLevel(level); }
	virtual void AddLoggingFunc(void (*pfnLogFunc)(const char* fileName, const char* accessType)) { FSTRACE("AddLoggingFunc"); m_real->AddLoggingFunc(pfnLogFunc); }
	virtual void RemoveLoggingFunc(FileSystemLoggingFunc_t logFunc) { FSTRACE("RemoveLoggingFunc"); m_real->RemoveLoggingFunc(logFunc); }
	virtual const FileSystemStatistics* GetFilesystemStatistics() { FSTRACE("GetFilesystemStatistics"); return m_real->GetFilesystemStatistics(); }

	virtual FileHandle_t OpenEx(const char* pFileName, const char* pOptions, unsigned flags, const char* pathID, char** ppszResolvedFilename) { FSTRACE("OpenEx"); return m_real->OpenEx(pFileName, pOptions, flags, pathID, ppszResolvedFilename); }
	virtual int ReadEx(void* pOutput, int sizeDest, int size, FileHandle_t file) { FSTRACE("ReadEx"); return m_real->ReadEx(pOutput, sizeDest, size, file); }
	virtual int ReadFileEx(const char* pFileName, const char* pPath, void** ppBuf, bool bNullTerminate, bool bOptimalAlloc, int nMaxBytes, int nStartingByte, FSAllocFunc_t pfnAlloc) { FSTRACE("ReadFileEx"); return m_real->ReadFileEx(pFileName, pPath, ppBuf, bNullTerminate, bOptimalAlloc, nMaxBytes, nStartingByte, pfnAlloc); }
	virtual FileNameHandle_t FindFileName(char const* pFileName) { FSTRACE("FindFileName"); return m_real->FindFileName(pFileName); }

	virtual void SetupPreloadData() { FSTRACE("SetupPreloadData"); m_real->SetupPreloadData(); }
	virtual void DiscardPreloadData() { FSTRACE("DiscardPreloadData"); m_real->DiscardPreloadData(); }
	virtual void LoadCompiledKeyValues(int type, char const* archiveFile) { FSTRACE("LoadCompiledKeyValues"); m_real->LoadCompiledKeyValues(type, archiveFile); }
	virtual KeyValues* LoadKeyValues(int type, char const* filename, char const* pPathID) { FSTRACE("LoadKeyValues(new)"); return m_real->LoadKeyValues(type, filename, pPathID); }
	virtual bool LoadKeyValues(KeyValues& head, int type, char const* filename, char const* pPathID) { FSTRACE("LoadKeyValues(into)"); return m_real->LoadKeyValues(head, type, filename, pPathID); }
	virtual bool ExtractRootKeyName(int type, char* outbuf, size_t bufsize, char const* filename, char const* pPathID) { FSTRACE("ExtractRootKeyName"); return m_real->ExtractRootKeyName(type, outbuf, bufsize, filename, pPathID); }

	virtual FSAsyncStatus_t AsyncWrite(const char* pFileName, const void* pSrc, int nSrcBytes, bool bFreeMemory, bool bAppend, FSAsyncControl_t* pControl) { FSTRACE("AsyncWrite"); return m_real->AsyncWrite(pFileName, pSrc, nSrcBytes, bFreeMemory, bAppend, pControl); }
	virtual FSAsyncStatus_t AsyncWriteFile(const char* pFileName, const CUtlBuffer* pSrc, int nSrcBytes, bool bFreeMemory, bool bAppend, FSAsyncControl_t* pControl) { FSTRACE("AsyncWriteFile"); return m_real->AsyncWriteFile(pFileName, pSrc, nSrcBytes, bFreeMemory, bAppend, pControl); }
	virtual FSAsyncStatus_t AsyncReadMultipleCreditAlloc(const FileAsyncRequest_t* pRequests, int nRequests, const char* pszFile, int line, FSAsyncControl_t* phControls) { FSTRACE("AsyncReadMultipleCreditAlloc"); return m_real->AsyncReadMultipleCreditAlloc(pRequests, nRequests, pszFile, line, phControls); }

	virtual bool GetFileTypeForFullPath(char const* pFullPath, wchar_t* buf, size_t bufSizeInBytes) { FSTRACE("GetFileTypeForFullPath"); return m_real->GetFileTypeForFullPath(pFullPath, buf, bufSizeInBytes); }
	virtual bool ReadToBuffer(FileHandle_t hFile, CUtlBuffer& buf, int nMaxBytes, FSAllocFunc_t pfnAlloc) { FSTRACE("ReadToBuffer"); return m_real->ReadToBuffer(hFile, buf, nMaxBytes, pfnAlloc); }

	virtual bool GetOptimalIOConstraints(FileHandle_t hFile, unsigned* pOffsetAlign, unsigned* pSizeAlign, unsigned* pBufferAlign) { FSTRACE("GetOptimalIOConstraints"); return m_real->GetOptimalIOConstraints(hFile, pOffsetAlign, pSizeAlign, pBufferAlign); }
	virtual void* AllocOptimalReadBuffer(FileHandle_t hFile, unsigned nSize, unsigned nOffset) { FSTRACE("AllocOptimalReadBuffer"); return m_real->AllocOptimalReadBuffer(hFile, nSize, nOffset); }
	virtual void FreeOptimalReadBuffer(void* p) { FSTRACE("FreeOptimalReadBuffer"); m_real->FreeOptimalReadBuffer(p); }

	virtual void BeginMapAccess() { FSTRACE("BeginMapAccess"); m_real->BeginMapAccess(); }
	virtual void EndMapAccess() { FSTRACE("EndMapAccess"); m_real->EndMapAccess(); }

	virtual bool FullPathToRelativePathEx(const char* pFullpath, const char* pPathId, char* pDest, int maxLenInChars) { FSTRACE("FullPathToRelativePathEx"); return m_real->FullPathToRelativePathEx(pFullpath, pPathId, pDest, maxLenInChars); }
	virtual int GetPathIndex(const FileNameHandle_t& handle) { FSTRACE("GetPathIndex"); return m_real->GetPathIndex(handle); }
	virtual long GetPathTime(const char* pPath, const char* pPathID) { FSTRACE("GetPathTime"); return m_real->GetPathTime(pPath, pPathID); }
	virtual DVDMode_t GetDVDMode() { FSTRACE("GetDVDMode"); return m_real->GetDVDMode(); }

	virtual void EnableWhitelistFileTracking(bool bEnable, bool bCacheAllVPKHashes, bool bRecalculateAndCheckHashes) { FSTRACE("EnableWhitelistFileTracking"); m_real->EnableWhitelistFileTracking(bEnable, bCacheAllVPKHashes, bRecalculateAndCheckHashes); }
	virtual void RegisterFileWhitelist(IPureServerWhitelist* pWhiteList, IFileList** pFilesToReload) { FSTRACE("RegisterFileWhitelist"); m_real->RegisterFileWhitelist(pWhiteList, pFilesToReload); }
	virtual void MarkAllCRCsUnverified() { FSTRACE("MarkAllCRCsUnverified"); m_real->MarkAllCRCsUnverified(); }
	virtual void CacheFileCRCs(const char* pPathname, ECacheCRCType eType, IFileList* pFilter) { FSTRACE("CacheFileCRCs"); m_real->CacheFileCRCs(pPathname, eType, pFilter); }
	virtual EFileCRCStatus CheckCachedFileHash(const char* pPathID, const char* pRelativeFilename, int nFileFraction, FileHash_t* pFileHash) { FSTRACE("CheckCachedFileHash"); return m_real->CheckCachedFileHash(pPathID, pRelativeFilename, nFileFraction, pFileHash); }
	virtual int GetUnverifiedFileHashes(CUnverifiedFileHash* pFiles, int nMaxFiles) { FSTRACE("GetUnverifiedFileHashes"); return m_real->GetUnverifiedFileHashes(pFiles, nMaxFiles); }
	virtual int GetWhitelistSpewFlags() { FSTRACE("GetWhitelistSpewFlags"); return m_real->GetWhitelistSpewFlags(); }
	virtual void SetWhitelistSpewFlags(int flags) { FSTRACE("SetWhitelistSpewFlags"); m_real->SetWhitelistSpewFlags(flags); }
	virtual void InstallDirtyDiskReportFunc(FSDirtyDiskReportFunc_t func) { FSTRACE("InstallDirtyDiskReportFunc"); m_real->InstallDirtyDiskReportFunc(func); }

	virtual FileCacheHandle_t CreateFileCache() { FSTRACE("CreateFileCache"); return m_real->CreateFileCache(); }
	virtual void AddFilesToFileCache(FileCacheHandle_t cacheId, const char** ppFileNames, int nFileNames, const char* pPathID) { FSTRACE("AddFilesToFileCache"); m_real->AddFilesToFileCache(cacheId, ppFileNames, nFileNames, pPathID); }
	virtual bool IsFileCacheFileLoaded(FileCacheHandle_t cacheId, const char* pFileName) { FSTRACE("IsFileCacheFileLoaded"); return m_real->IsFileCacheFileLoaded(cacheId, pFileName); }
	virtual bool IsFileCacheLoaded(FileCacheHandle_t cacheId) { FSTRACE("IsFileCacheLoaded"); return m_real->IsFileCacheLoaded(cacheId); }
	virtual void DestroyFileCache(FileCacheHandle_t cacheId) { FSTRACE("DestroyFileCache"); m_real->DestroyFileCache(cacheId); }

	virtual bool RegisterMemoryFile(CMemoryFileBacking* pFile, CMemoryFileBacking** ppExistingFileWithRef) { FSTRACE("RegisterMemoryFile"); return m_real->RegisterMemoryFile(pFile, ppExistingFileWithRef); }
	virtual void UnregisterMemoryFile(CMemoryFileBacking* pFile) { FSTRACE("UnregisterMemoryFile"); m_real->UnregisterMemoryFile(pFile); }

	virtual void CacheAllVPKFileHashes(bool bCacheAllVPKHashes, bool bRecalculateAndCheckHashes) { FSTRACE("CacheAllVPKFileHashes"); m_real->CacheAllVPKFileHashes(bCacheAllVPKHashes, bRecalculateAndCheckHashes); }
	virtual bool CheckVPKFileHash(int PackFileID, int nPackFileNumber, int nFileFraction, MD5Value_t& md5Value) { FSTRACE("CheckVPKFileHash"); return m_real->CheckVPKFileHash(PackFileID, nPackFileNumber, nFileFraction, md5Value); }
	virtual void NotifyFileUnloaded(const char* pszFilename, const char* pPathId) { FSTRACE("NotifyFileUnloaded"); m_real->NotifyFileUnloaded(pszFilename, pPathId); }

	virtual void RemoveSearchPathsByGroup(int group) { FSTRACE("RemoveSearchPathsByGroup"); m_real->RemoveSearchPathsByGroup(group); }
	virtual void SetGet(void* pGet) { FSTRACE("SetGet"); m_real->SetGet(pGet); }
	virtual void* Addons() { FSTRACE("Addons"); return m_real->Addons(); }
	virtual void* Gamemodes() { FSTRACE("Gamemodes"); return m_real->Gamemodes(); }
	virtual void* Games() { FSTRACE("Games"); return m_real->Games(); }
	virtual void* LegacyAddons() { FSTRACE("LegacyAddons"); return m_real->LegacyAddons(); }
	virtual void* Language() { FSTRACE("Language"); return m_real->Language(); }
	virtual void DoFilesystemRefresh() { FSTRACE("DoFilesystemRefresh"); m_real->DoFilesystemRefresh(); }
	virtual int LastFilesystemRefresh() { FSTRACE("LastFilesystemRefresh"); return m_real->LastFilesystemRefresh(); }
	virtual void AddVPKFileFromPath(const char* pPath, const char* pPathID, unsigned int addType) { FSTRACE("AddVPKFileFromPath"); m_real->AddVPKFileFromPath(pPath, pPathID, addType); }
	virtual void GMOD_SetupDefaultPaths(const char* pFullPath, const char* pPathID) { FSTRACE("GMOD_SetupDefaultPaths"); m_real->GMOD_SetupDefaultPaths(pFullPath, pPathID); }
	virtual void GMOD_FixPathCase(char* pPath, size_t maxLen) { FSTRACE("GMOD_FixPathCase"); m_real->GMOD_FixPathCase(pPath, maxLen); }

private:
	CLoggingFileSystem* m_real;
};

#undef FSTRACE

InstantiateInterfaceFn g_realFileSystemFactory = nullptr;
CLoggingFileSystem* g_loggingFileSystem = nullptr;

CLoggingFileSystem* AcquireProxy() {
	if (!g_loggingFileSystem && g_realFileSystemFactory) {
		auto* real = reinterpret_cast<CLoggingFileSystem*>(g_realFileSystemFactory());
		if (real) {
			g_loggingFileSystem = new CLoggingFileSystem(real);
			Log("example3", "installed logging filesystem proxy %p wrapping real %p\n", g_loggingFileSystem, real);
		}
	}
	return g_loggingFileSystem;
}

void* CreateLoggingFileSystem() {
	return static_cast<IAppSystem*>(AcquireProxy());
}

void* CreateLoggingBaseFileSystem() {
	return static_cast<IBaseFileSystemGmod*>(AcquireProxy());
}

void RewriteFileSystemInterfaces(InterfaceRegistryEditor& editor) {
	InterfaceRegistration fs = editor.GetInterface(FILESYSTEM_INTERFACE_VERSION);
	if (fs.Valid) {
		if (!g_realFileSystemFactory)
			g_realFileSystemFactory = fs.Curr->m_CreateFn;
		fs.Reroute(&CreateLoggingFileSystem);
	}

	InterfaceRegistration bfs = editor.GetInterface(BASEFILESYSTEM_INTERFACE_VERSION);
	if (bfs.Valid)
		bfs.Reroute(&CreateLoggingBaseFileSystem);
}

MODULE_MAIN() {
	api->Log("example3", "Loaded Example Module #3: Interface Writing (abi %u)\n", api->abiVersion);
	api->AddInterfaceRewriter(&RewriteFileSystemInterfaces);
} MODULE_END()