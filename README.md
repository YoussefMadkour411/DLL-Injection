# DLL-Injection

Similar to pe-injection, but instead of injecting raw shell-code, it injects a DLL file.

Allocates memory in the target process and writes the full path (name) of the malicious DLL.

Uses VirtualAllocEx and WriteProcessMemory.

Creates a remote thread to load the DLL into the target process.

Uses CreateRemoteThread.

The lpStartAddress points to LoadLibraryA in the target process.

The lpParameter points to the address where the DLL name was written.

Demo:

![image](https://github.com/user-attachments/assets/41b9319b-0466-4172-a50b-46ce380e06e4)

![image](https://github.com/user-attachments/assets/849543b2-5f6b-44d8-b4ca-20fedb422de7)
