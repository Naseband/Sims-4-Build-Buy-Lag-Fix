#include "pch.h"
#include <Windows.h>
#include <stdio.h>

#include "NINI.h"

// These addresses are for 1.87.40.1030 - only used if the ini does not exist or does not contain the respective entries.

constexpr long long PATCH_ADDRESS =     0x0140c8083d;
constexpr long long PATCH_EXPECTED =    0xE081C71774;
constexpr long long PATCH_TARGET =      0xE081C717EB;

void Patch()
{
    char
        szText[150];

    unsigned long long
        ullData;

    NINI
        *pNini = new NINI();

    long long
        llAddress = PATCH_ADDRESS,
        llExpected = PATCH_EXPECTED,
        llTarget = PATCH_TARGET;

    bool
        bShowErrors = true,
        bShowWarnings = true,
        bShowSuccess = true,

        bExitAfterPatch = false;

    bool
        bMalformedAddress = false,
        bMalformedExpected = false,
        bMalformedTarget = false;

    // Load ini (either from scripts or root)

    if (!pNini->Load("BBLagFix.ini"))
        pNini->Load("../BBLagFix.ini");

    if (pNini->IsLoaded()) // If the file was parsed load the data
    {
        // Patch

        if (pNini->GetKeyValueByName("Address", szText, sizeof(szText), "Patch") &&
            sscanf_s(szText, "%llx", &llAddress) != 1)
            bMalformedAddress = true;

        if (pNini->GetKeyValueByName("Expected", szText, sizeof(szText), "Patch") &&
            sscanf_s(szText, "%llx", &llExpected) != 1)
            bMalformedExpected = true;

        if (pNini->GetKeyValueByName("Target", szText, sizeof(szText), "Patch") &&
            sscanf_s(szText, "%llx", &llTarget) != 1)
            bMalformedTarget = true;

        // Messages

        pNini->GetKeyValueByName("Errors", bShowErrors, "Messages");
        pNini->GetKeyValueByName("Warnings", bShowWarnings, "Messages");
        pNini->GetKeyValueByName("Success", bShowSuccess, "Messages");

        // Debug

        pNini->GetKeyValueByName("ExitAfterPatch", bExitAfterPatch, "Debug");
    }

    // Check if some of the patch data was invalid and output an error, if enabled

    if (bShowErrors)
    {
        if (bMalformedAddress)
            MessageBoxA(NULL, "ERROR:\nAddress value in configuration file is not a valid hex value.", "BBLagFix", 0);

        if (bMalformedExpected)
            MessageBoxA(NULL, "ERROR:\nExpected value in configuration file is not a valid hex value.", "BBLagFix", 0);

        if (bMalformedTarget)
            MessageBoxA(NULL, "ERROR:\nTarget value in configuration file is not a valid hex value.", "BBLagFix", 0);
    }

    // Check again for the patch itself

    if (!bMalformedAddress && !bMalformedExpected && !bMalformedTarget)
    {
        // Get the current data from the memory address

        ullData = *(unsigned long long*)llAddress;

        if (ullData != llExpected && ullData != llTarget) // If the target data is neither the expected value, nor the patched value something went wrong
        {
            if (bShowErrors)
            {
                snprintf(szText, sizeof(szText), "ERROR:\nFailed to apply BBLagFix patch.\nExpected data: %llx\nActual data: %llx\nMake sure to use the correct version.", llExpected, ullData);
                MessageBoxA(NULL, szText, "BBLagFix", 0);
            }
        }
        else if (ullData == llTarget) // Already patched?
        {
            if (bShowWarnings)
            {
                MessageBoxA(NULL, "WARNING:\nBBLagFix was already applied.", "BBLagFix", 0);
            }
        }
        else // Patch...
        {
            DWORD
                dwOldProt;

            VirtualProtect((VOID*)llAddress, sizeof(unsigned long long), PAGE_EXECUTE_READWRITE, &dwOldProt);

            *(unsigned long long*)llAddress = (unsigned)llTarget;

            VirtualProtect((VOID*)llAddress, sizeof(unsigned long long), dwOldProt, &dwOldProt);

            if (bShowSuccess)
            {
                MessageBoxA(NULL, "SUCCESS:\nBBLagFix applied successfully.", "BBLagFix", 0);
            }
        }
    }

    // Debug

    if(bExitAfterPatch)
        ExitProcess(0);

    delete pNini;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:

        Patch();

        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}
