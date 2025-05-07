#include <iostream>
#include <Windows.h>

using namespace std;

int main(int argc, char *argv[])
{
    // Check if the correct number of arguments is passed
    if (argc != 2)
    {
        cout << "Usage: " << argv[0] << " <PID>" << endl;
        return 1;
    }

    DWORD TiD = 0;          // Thread ID of the remote thread created in the target process
    char dllPath[MAX_PATH]; // Buffer to hold the full DLL path

    // Get the full absolute path of the DLL
    if (!GetFullPathNameA("./kike.dll", MAX_PATH, dllPath, nullptr))
    {
        cout << "[-] Failed to get full path of the DLL. Error: " << GetLastError() << endl;
        return 1;
    }

    cout << "Injecting DLL into process " << atoi(argv[1]) << endl;

    // Open the target process with necessary access rights
    HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, atoi(argv[1]));
    if (!processHandle)
    {
        cout << "[-] Failed to open process. Error: " << GetLastError() << endl;
        return -1;
    }

    // Allocate memory in the remote process for the DLL path
    LPVOID remoteBuffer = VirtualAllocEx(processHandle, NULL, strlen(dllPath) + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!remoteBuffer)
    {
        cout << "[-] Memory allocation failed. Error: " << GetLastError() << endl;
        CloseHandle(processHandle);
        return -1;
    }

    // Write the DLL path to the allocated memory in the remote process
    if (!WriteProcessMemory(processHandle, remoteBuffer, dllPath, strlen(dllPath) + 1, nullptr))
    {
        cout << "[-] Failed to write memory. Error: " << GetLastError() << endl;
        VirtualFreeEx(processHandle, remoteBuffer, 0, MEM_RELEASE);
        CloseHandle(processHandle);
        return -1;
    }

    // Get the handle to Kernel32.dll, which contains LoadLibraryA
    HMODULE hKernel32 = GetModuleHandleA("Kernel32.dll");
    if (!hKernel32)
    {
        cout << "[-] GetModuleHandleA failed. Error: " << GetLastError() << endl;
        VirtualFreeEx(processHandle, remoteBuffer, 0, MEM_RELEASE);
        CloseHandle(processHandle);
        return -1;
    }

    // Get the address of LoadLibraryA function
    FARPROC loadLibraryAddr = GetProcAddress(hKernel32, "LoadLibraryA");
    if (!loadLibraryAddr)
    {
        cout << "[-] GetProcAddress failed. Error: " << GetLastError() << endl;
        VirtualFreeEx(processHandle, remoteBuffer, 0, MEM_RELEASE);
        CloseHandle(processHandle);
        return -1;
    }

    // Create a remote thread in the target process to execute LoadLibraryA with our DLL path
    HANDLE ThreadHandle = CreateRemoteThread(processHandle, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddr, remoteBuffer, 0, &TiD);
    if (!ThreadHandle)
    {
        cout << "[-] Failed to create remote thread. Error: " << GetLastError() << endl;
        VirtualFreeEx(processHandle, remoteBuffer, 0, MEM_RELEASE);
        CloseHandle(processHandle);
        return -1;
    }

    cout << "[+] Remote thread created. Thread ID: " << TiD << endl;

    // Cleanup: Free allocated memory and close handles
    VirtualFreeEx(processHandle, remoteBuffer, 0, MEM_RELEASE);
    CloseHandle(ThreadHandle);
    CloseHandle(processHandle);

    return 0;
}
