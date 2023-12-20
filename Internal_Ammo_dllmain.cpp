#include "pch.h"
#include "iostream"

DWORD ac_client_addr = 0x0;
DWORD ammo_addr      = 0x0;
DWORD ammo_dec_addr  = 0x0;
bool ammo_freeze     = false;
bool ammo_patch      = false;


void Patch(DWORD addr, const char* newCode, unsigned int size) {
    DWORD oldProtect;
    VirtualProtect((BYTE*)addr, size, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy((BYTE*)addr, newCode, size);
    VirtualProtect((BYTE*)addr, size, oldProtect, &oldProtect);
}

void NOP(DWORD addr, unsigned int size) {
    char* NOParray = new char[size];
    memset(NOParray, 0x90, size);

    Patch(addr, NOParray, size);
    delete[] NOParray;
}

void PrintHackMenu(DWORD proc_id) {
    printf("AssaultCube - Internal\n");
    printf("======================\n");
    printf("Process id: %d\n\n", proc_id);
    printf("[NUMPAD 0] - Freeze/Unfreeze Ammo\n");
    printf("[NUMPAD 1] - Patch/Unpatch Ammo\n");
    printf("[NUMPAD 9] - Exit\n\n");
    printf("Log:\n");
}

void HackThread(HMODULE hModule) {
    // Console
    AllocConsole();
    FILE *f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    DWORD process_id = GetCurrentProcessId();
    PrintHackMenu(process_id);

    // Addresses
    ac_client_addr = (DWORD)GetModuleHandle(L"ac_client.exe");  // image addr of game
    ammo_addr = *(DWORD*)(ac_client_addr + 0x00109B74) + 0x150;
    ammo_dec_addr = ac_client_addr + 0x000637E9;

    // Loop
    while (true) {
        // Freeze
        if (GetAsyncKeyState(VK_NUMPAD0) & 1) {
            ammo_freeze = !ammo_freeze;
            if (ammo_freeze) {
                printf("Freeze - ON\n");
            }
            else {
                printf("Freeze - OFF\n");
            } 
        }

        // Set Ammo to 1337
        if (ammo_freeze) {
            *(DWORD*)ammo_addr = 1337;
        }

        // Patch
        if (GetAsyncKeyState(VK_NUMPAD1) & 1) {
            ammo_patch = !ammo_patch;
            if (ammo_patch) {
                NOP(ammo_dec_addr, 2);
            }
            else {
                Patch(ammo_dec_addr, "\xFF\x0E", 2);
            }
        }

        // Close Internal
        if (GetAsyncKeyState(VK_NUMPAD9) & 1) {
            break;
        }

        Sleep(10);
    }

    // Cleanup
    fclose(f);
    FreeConsole();
    FreeLibraryAndExitThread(hModule, 0);
}

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {

    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)HackThread, hModule, 0, 0);
        break;
    }

    return TRUE;
}

