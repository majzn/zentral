#if 0
#include <stdio.h>
#include <windows.h>
#include <time.h>

int main() {
  SYSTEMTIME st;
  GetSystemTime(&st);
  char currentTime[512];
  sprintf(currentTime,"[%d/%d/%d  %d:%d:%d %d] - ",
        st.wDay,st.wMonth,st.wYear, st.wHour,
        st.wMinute, st.wSecond , st.wMilliseconds);
  printf("%s IP Acquired...\n", currentTime);
  Sleep(3145);
  return 0;
}
#endif

#if 0
#include <netdb.h>
#include <arpa/inet.h>

int main()
{
  const char* const host = "thecomputername" ;
  const hostent* host_info = 0 ;
  host_info = gethostbyname(host) ;

  if(host_info)
  {
    std::cout << "host: " << host_info->h_name << '\n' ;

    for( int i=0 ; host_info->h_addr_list[i] ; ++i )
    {
      const in_addr* address = (in_addr*)host_info->h_addr_list[i] ;
      std::cout << " address: " << inet_ntoa( *address ) << '\n' ;
    }
  }
  else herror( "error" ) ;
}
#endif

#include <time.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

// Link with Iphlpapi.lib
#pragma comment(lib, "IPHLPAPI.lib")

#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

/* Note: could also use malloc() and free() */

int __cdecl main(int argc, char **argv) {
    /* Declare and initialize variables */
    DWORD dwRetVal = 0;

    int i = 0;

    // Set the flags to pass to GetAdaptersAddresses
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;

    // default to unspecified address family (both)
    ULONG family = AF_UNSPEC;

    LPVOID lpMsgBuf = NULL;

    PIP_ADAPTER_ADDRESSES pAddresses = NULL;
    ULONG outBufLen = 0;
    ULONG Iterations = 0;

    PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;

    family = AF_INET;

    // Allocate a 15 KB buffer to start with.
    outBufLen = WORKING_BUFFER_SIZE;

    Sleep(200);
    SYSTEMTIME st;
    GetSystemTime(&st);
    char currentTime[512];
    sprintf(currentTime,"[%d/%d/%d  %d:%d:%d %d] - ",
        st.wDay,st.wMonth,st.wYear, st.wHour,
        st.wMinute, st.wSecond , st.wMilliseconds);
    printf("%s IP Acquired...\n", currentTime);
    Sleep(2000);
    
    do {
        Sleep(50);
        pAddresses = (IP_ADAPTER_ADDRESSES *) MALLOC(outBufLen);
        if (pAddresses == NULL) {
            printf
              ("Memory allocation failed for IP_ADAPTER_ADDRESSES struct\n");
            exit(1);
        }

        dwRetVal =
            GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen);

        if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
            FREE(pAddresses);
            pAddresses = NULL;
        } else {
            break;
        }

        Iterations++;

    } while ((dwRetVal == ERROR_BUFFER_OVERFLOW) && (Iterations < MAX_TRIES));

    if (dwRetVal == NO_ERROR) {
        // If successful, output some information from the data we received
        pCurrAddresses = pAddresses;
        while (pCurrAddresses) {
            Sleep(10);
            printf("\tAdapter name: %s\n", pCurrAddresses->AdapterName);
            Sleep(40);
            printf("\tDNS Suffix: %wS\n", pCurrAddresses->DnsSuffix);
            Sleep(10);
            printf("\tDescription: %wS\n", pCurrAddresses->Description);
            Sleep(1);
            printf("\tFriendly name: %wS\n", pCurrAddresses->FriendlyName);
            Sleep(50);
            if (pCurrAddresses->PhysicalAddressLength != 0) {
                printf("\tPhysical address: ");
                for (i = 0; i < (int)pCurrAddresses->PhysicalAddressLength;
                     i++) {
                    Sleep(5);
                    if (i == (int)(pCurrAddresses->PhysicalAddressLength - 1))
                        printf("%d\n",
                               (int) pCurrAddresses->PhysicalAddress[i]);
                    else
                        printf("%d-",
                               (int) pCurrAddresses->PhysicalAddress[i]);
                }
            }
            printf("\n");
            pCurrAddresses = pCurrAddresses->Next;
        }
    } else {
        printf("Call to GetAdaptersAddresses failed with error: %d\n",
               (int)dwRetVal);
        if (dwRetVal == ERROR_NO_DATA)
            printf("\tNo addresses were found for the requested parameters\n");
        else {
            if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
                    NULL, dwRetVal, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),   
                    // Default language
                    (LPTSTR) & lpMsgBuf, 0, NULL)) {
                printf("\tError: %s", (char*)lpMsgBuf);
                LocalFree(lpMsgBuf);
                if (pAddresses)
                    FREE(pAddresses);
                exit(1);
            }
        }
    }
    if(pAddresses) {
      FREE(pAddresses);
    }
    Sleep(200);
    return 0;
}
