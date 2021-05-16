#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <winsock2.h>
#include <ws2spi.h>
#include <tchar.h>

#include <vector>
#include <unordered_map>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "User32.lib")

WSPPROC_TABLE NextProcTable;
static char exePath[260];

BOOL WINAPI DllMain(
	HINSTANCE hinstDLL,  // handle to DLL module
	DWORD fdwReason,     // reason for calling function
	LPVOID lpReserved)  // reserved
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
	{
		//获取当前加载本DLL的进程路径
		GetModuleFileNameA(NULL, exePath, 260);

		OutputDebugStringA(exePath);
	}
		break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

bool filter(SOCKET s)
{
	bool isFirewallOpen = GetPrivateProfileIntA("Firewall", "Status", 0, "C:\\Windows\\FW_rule.ini");
	if (!isFirewallOpen)
		return false;

	bool isBlockAll = GetPrivateProfileIntA("Firewall", "BlockAll", 0, "C:\\Windows\\FW_rule.ini");
	bool isBlockThis = GetPrivateProfileIntA(exePath, "isBlock", 0, "C:\\Windows\\FW_rule.ini");

	HANDLE hMailslot;
	hMailslot = CreateFileA("\\\\.\\mailslot\\FirewallMailSlot", 
		GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	char err[50] = { 0 };
	sprintf_s(err, 50, ">>>>>>>>>%ld<<<<<<<", GetLastError());
	OutputDebugStringA(err);

	if (hMailslot != INVALID_HANDLE_VALUE)
	{
		SYSTEMTIME sys;
		GetLocalTime(&sys);
		char time[50] = { 0 };
		sprintf_s(time, 50, "%4d/%02d/%02d %02d:%02d:%02d", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond);

		std::string buf;
		buf += time;
		buf += "\"";
		buf += exePath;
		buf += "\"";
		buf += (isBlockAll || isBlockThis) ? "尝试联网，已拦截！" : "尝试联网，已允许！";
		OutputDebugStringA(buf.c_str());
		WriteFile(hMailslot, buf.c_str(), buf.size(), NULL, NULL);
	}
	CloseHandle(hMailslot);

	return (isBlockAll || isBlockThis);
}

int WSPAPI WSPSend(
	SOCKET                 s,
	LPWSABUF               lpBuffers,
	DWORD                  dwBufferCount,
	LPDWORD                lpNumberOfBytesSent,
	DWORD                  dwFlags,
	LPWSAOVERLAPPED        lpOverlapped,
	LPWSAOVERLAPPED_COMPLETION_ROUTINE   lpCompletionRoutine,
	LPWSATHREADID          lpThreadId,
	LPINT                  lpErrno
)
{
	if (filter(s))
	{
		//OutputDebugStringA("断开连接！");
		return SOCKET_ERROR;
	}
	return NextProcTable.lpWSPSend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
}

int WSPAPI WSPSendTo(
	SOCKET                        s,
	LPWSABUF                      lpBuffers,
	DWORD                         dwBufferCount,
	LPDWORD                       lpNumberOfBytesSent,
	DWORD                         dwFlags,
	const struct sockaddr FAR     *lpTo,
	int                           iTolen,
	LPWSAOVERLAPPED               lpOverlapped,
	LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
	LPWSATHREADID                 lpThreadId,
	LPINT                         lpErrno
)
{
	if (filter(s))
	{
		//OutputDebugStringA("断开连接！");
		return SOCKET_ERROR;
	}
	return NextProcTable.lpWSPSendTo(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpTo, iTolen, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
}

int WSPAPI WSPRecv(
	SOCKET               s,
	LPWSABUF             lpBuffers,
	DWORD                dwBufferCount,
	LPDWORD              lpNumberOfBytesRecvd,
	LPDWORD              lpFlags,
	LPWSAOVERLAPPED      lpOverlapped,
	LPWSAOVERLAPPED_COMPLETION_ROUTINE   lpCompletionRoutine,
	LPWSATHREADID        lpThreadId,
	LPINT                lpErrno
)
{
	if (filter(s))
	{
		//OutputDebugStringA("断开连接！");
		return SOCKET_ERROR;
	}
	return NextProcTable.lpWSPRecv(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
}

int WSPAPI WSPRecvFrom(
	SOCKET                  s,
	LPWSABUF                lpBuffers,
	DWORD                   dwBufferCount,
	LPDWORD                 lpNumberOfBytesRecvd,
	LPDWORD                 lpFlags,
	struct sockaddr FAR     *lpFrom,
	LPINT                   lpFromlen,
	LPWSAOVERLAPPED         lpOverlapped,
	LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
	LPWSATHREADID           lpThreadId,
	LPINT                   lpErrno
)
{
	if (filter(s))
	{
		//OutputDebugStringA("断开连接！");
		return SOCKET_ERROR;
	}
	return NextProcTable.lpWSPRecvFrom(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpFrom, lpFromlen, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
}

SOCKET WSPAPI WSPSocket(
	int                    af,
	int                    type,
	int                    protocol,
	LPWSAPROTOCOL_INFOW    lpProtocolInfo,
	GROUP                  g,
	DWORD                  dwFlags,
	LPINT                  lpErrno
)
{
	//OutputDebugStringA("WSPSocket run!");
	return NextProcTable.lpWSPSocket(af, type, protocol, lpProtocolInfo, g, dwFlags, lpErrno);
}

bool GetHookProvider(LPWSAPROTOCOL_INFOW lpProtocolInfo, TCHAR *tcLibraryPath)
{
	//获取原服务提供者的路径

	HKEY hKey;
	
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Services\\WinSock2\\Firewall"), 0, KEY_READ, &hKey)!=ERROR_SUCCESS)
	{
		OutputDebugStringA("Cannot open key!");
		return false;
	}

	TCHAR *nameBuf = new TCHAR[MAX_PATH];
	BYTE *dataBuf = new BYTE[1024];
	DWORD szData = 1024, szName = MAX_PATH, dwIndex = 0;

	//OutputDebugStringA("GetHookProvider run!");
	while (RegEnumValue(hKey, dwIndex, nameBuf, &szName, NULL, NULL, dataBuf, &szData) != ERROR_NO_MORE_ITEMS)
	{
		OutputDebugString(nameBuf);
		if (lpProtocolInfo->dwCatalogEntryId == _wtoi(nameBuf))
		{
			memcpy(tcLibraryPath, dataBuf, MAX_PATH);
			OutputDebugString(tcLibraryPath);
			ExpandEnvironmentStrings(tcLibraryPath, tcLibraryPath, MAX_PATH);
			OutputDebugString(tcLibraryPath);
			return true;
		}
		szName = MAX_PATH;
		dwIndex++;
	}
	//OutputDebugStringA("GetHookProvider return false!");
	return false;
}

int WSPAPI WSPStartup(
	WORD wVersionRequessted,
	LPWSPDATA lpWSPData,
	LPWSAPROTOCOL_INFOW lpProtocolInfo,
	WSPUPCALLTABLE upcallTable,
	LPWSPPROC_TABLE lpProcTable
)
{
	TCHAR tcLibraryPath[512];
	ZeroMemory(tcLibraryPath,512);
	LPWSPSTARTUP WSPStartupFunc = NULL;
	HMODULE hLibraryHandle = NULL;
	int ErrorCode = 0;

	//OutputDebugStringA("WSPStartup start");

	if ( !GetHookProvider(lpProtocolInfo, tcLibraryPath)
		|| ((hLibraryHandle = LoadLibrary(tcLibraryPath)) == NULL)
		|| ((WSPStartupFunc = (LPWSPSTARTUP)GetProcAddress(hLibraryHandle, "WSPStartup")) == NULL))
	{
		if (hLibraryHandle == NULL)
		{
			OutputDebugStringA("handle NULL!");
		}
		if (WSPStartupFunc == NULL)
		{
			OutputDebugStringA("Func NULL!");
		}
		/*int err = GetLastError();
		TCHAR errStr[5];
		_itow(err, errStr, 10);
		OutputDebugStringW(errStr);*/
		//OutputDebugStringA("WSPStartup return WSAEPROVIDERFAILEDINIT");
		return WSAEPROVIDERFAILEDINIT;
	}
		

	if ((ErrorCode = WSPStartupFunc(wVersionRequessted, lpWSPData, lpProtocolInfo, upcallTable, lpProcTable)) != ERROR_SUCCESS)
	{
		//OutputDebugStringA("WSPStartupFunc return ErrorCode");
		return ErrorCode;
	}

	NextProcTable = *lpProcTable;
	//OutputDebugStringA("WSPStartupFunc return ErrorCode");

	//lpProcTable->lpWSPSocket = WSPSocket;
	lpProcTable->lpWSPSend = WSPSend;
	lpProcTable->lpWSPSendTo = WSPSendTo;
	lpProcTable->lpWSPRecv = WSPRecv;
	lpProcTable->lpWSPRecvFrom = WSPRecvFrom;

	//OutputDebugStringA("WSPStratup end");
	return 0;
}
