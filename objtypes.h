#ifndef __OBJTYPES_H_VER__
#define __OBJTYPES_H_VER__ 2016032200
// $Id$
#if (defined(_MSC_VER) && (_MSC_VER >= 1020)) || defined(__MCPP)
#pragma once
#endif // Check for "#pragma once" support

#define IDI_UNKNOWN_OBJTYPE      5000
#define IDI_ADAPTER              5001
#define IDI_CALLBACK             5002
#define IDI_COMPOSITIONSURFACE   5003
#define IDI_CONTROLLER           5004
#define IDI_DEBUGOBJECT          5005
#define IDI_DESKTOP              5006
#define IDI_DEVICE               5007
#define IDI_DIRECTORY            5008
#define IDI_DRIVER               5009
#define IDI_EVENT                5010
#define IDI_EVENTPAIR            5011
#define IDI_FILE                 5012
#define IDI_FILTERCONNECTIONPORT 5013
#define IDI_IOCOMPLETION         5014
#define IDI_JOB                  5015
#define IDI_KEY                  5016
#define IDI_KEYEDEVENT           5017
#define IDI_MUTANT               5018
#define IDI_PCWOBJECT            5019
#define IDI_PORT                 5020
#define IDI_POWERREQUEST         5021
#define IDI_PROCESS              5022
#define IDI_SECTION              5023
#define IDI_SEMAPHORE            5024
#define IDI_SESSION              5025
#define IDI_SYMBOLICLINK         5026
#define IDI_THREAD               5027
#define IDI_TIMER                5028
#define IDI_TMEN                 5029
#define IDI_TOKEN                5030
#define IDI_TYPE                 5031
#define IDI_USERAPCRESERVE       5032
#define IDI_WAITABLEPORT         5033
#define IDI_WAITCOMPLETIONPKT    5034
#define IDI_WINDOWSTATION        5035
#define IDI_WMIGUID              5036

// Not strictly object types in the object manager sense
#define IDI_MAILSLOT             5100
#define IDI_PIPE                 5101

// Special types
#define IDI_OBJMGR_ROOT          5900
#define IDI_SYMLINK_TO_DIRECTORY 5901


// "Aliases" (i.e. we reuse the images) of the above types
#define IDI_ALPCPORT             IDI_PORT
#define IDI_FILTERCOMMUNICATIONPORT IDI_FILTERCONNECTIONPORT
#define IDI_TMRM                 IDI_TMEN
#define IDI_TMTM                 IDI_TMEN
#define IDI_TMTX                 IDI_TMEN

#define OBJTYPESTR_ADAPTER              L"Adapter"
#define OBJTYPESTR_CALLBACK             L"Callback"
#define OBJTYPESTR_COMPOSITIONSURFACE   L"CompositionSurface"
#define OBJTYPESTR_CONTROLLER           L"Controller"
#define OBJTYPESTR_DEBUGOBJECT          L"DebugObject"
#define OBJTYPESTR_DESKTOP              L"Desktop"
#define OBJTYPESTR_DEVICE               L"Device"
#define OBJTYPESTR_DIRECTORY            L"Directory"
#define OBJTYPESTR_DRIVER               L"Driver"
#define OBJTYPESTR_EVENT                L"Event"
#define OBJTYPESTR_EVENTPAIR            L"EventPair"
#define OBJTYPESTR_FILE                 L"File"
#define OBJTYPESTR_FILTERCONNECTIONPORT L"FilterConnectionPort"
#define OBJTYPESTR_IOCOMPLETION         L"IoCompletion"
#define OBJTYPESTR_JOB                  L"Job"
#define OBJTYPESTR_KEY                  L"Key"
#define OBJTYPESTR_KEYEDEVENT           L"KeyedEvent"
#define OBJTYPESTR_MUTANT               L"Mutant"
#define OBJTYPESTR_PCWOBJECT            L"PcwObject"
#define OBJTYPESTR_PORT                 L"Port"
#define OBJTYPESTR_POWERREQUEST         L"PowerRequest"
#define OBJTYPESTR_PROCESS              L"Process"
#define OBJTYPESTR_SECTION              L"Section"
#define OBJTYPESTR_SEMAPHORE            L"Semaphore"
#define OBJTYPESTR_SESSION              L"Session"
#define OBJTYPESTR_SYMBOLICLINK         L"SymbolicLink"
#define OBJTYPESTR_THREAD               L"Thread"
#define OBJTYPESTR_TIMER                L"Timer"
#define OBJTYPESTR_TMEN                 L"TmEn"
#define OBJTYPESTR_TOKEN                L"Token"
#define OBJTYPESTR_TYPE                 L"Type"
#define OBJTYPESTR_USERAPCRESERVE       L"UserApcReserve"
#define OBJTYPESTR_WAITABLEPORT         L"WaitablePort"
#define OBJTYPESTR_WAITCOMPLETIONPKT    L"WaitCompletionPacket"
#define OBJTYPESTR_WINDOWSTATION        L"WindowStation"
#define OBJTYPESTR_WMIGUID              L"WmiGuid"
#define OBJTYPESTR_MAILSLOT             L"Mailslot"
#define OBJTYPESTR_PIPE                 L"Pipe"

// "Aliases" (i.e. we reuse the images) of the above types
#define OBJTYPESTR_ALPCPORT             L"ALPC Port"
#define OBJTYPESTR_FILTERCOMMUNICATIONPORT L"FilterCommunicationPort"
#define OBJTYPESTR_TMRM                 L"TmRm"
#define OBJTYPESTR_TMTM                 L"TmTm"
#define OBJTYPESTR_TMTX                 L"TmTx"

#define OBJTYPESTR_UNKNOWN_OBJTYPE      L"Unknown object type"

#ifndef RC_INVOKED
#define TBL_ENTRY(x) IDI_##x, OBJTYPESTR_##x
struct { WORD resId; LPCWSTR typeName; } resIconTypeMapping[] =
{
    TBL_ENTRY(UNKNOWN_OBJTYPE),
    TBL_ENTRY(ADAPTER),
    TBL_ENTRY(CALLBACK),
    TBL_ENTRY(COMPOSITIONSURFACE),
    TBL_ENTRY(CONTROLLER),
    TBL_ENTRY(DEBUGOBJECT), // 5
    TBL_ENTRY(DESKTOP),
    TBL_ENTRY(DEVICE),
    TBL_ENTRY(DIRECTORY),
    TBL_ENTRY(DRIVER),
    TBL_ENTRY(EVENT), // 10
    TBL_ENTRY(EVENTPAIR),
    TBL_ENTRY(FILE),
    TBL_ENTRY(FILTERCONNECTIONPORT),
    TBL_ENTRY(IOCOMPLETION),
    TBL_ENTRY(JOB), // 15
    TBL_ENTRY(KEY),
    TBL_ENTRY(KEYEDEVENT),
    TBL_ENTRY(MUTANT),
    TBL_ENTRY(PCWOBJECT),
    TBL_ENTRY(PORT), // 20
    TBL_ENTRY(POWERREQUEST),
    TBL_ENTRY(PROCESS),
    TBL_ENTRY(SECTION),
    TBL_ENTRY(SEMAPHORE),
    TBL_ENTRY(SESSION), // 25
    TBL_ENTRY(SYMBOLICLINK),
    TBL_ENTRY(THREAD),
    TBL_ENTRY(TIMER),
    TBL_ENTRY(TMEN),
    TBL_ENTRY(TOKEN), // 30
    TBL_ENTRY(TYPE),
    TBL_ENTRY(USERAPCRESERVE),
    TBL_ENTRY(WAITABLEPORT),
    TBL_ENTRY(WAITCOMPLETIONPKT),
    TBL_ENTRY(WINDOWSTATION), // 35
    TBL_ENTRY(WMIGUID),
    TBL_ENTRY(ALPCPORT),
    TBL_ENTRY(FILTERCOMMUNICATIONPORT),
    TBL_ENTRY(TMRM),
    TBL_ENTRY(TMTM), // 40
    TBL_ENTRY(TMTX),
    TBL_ENTRY(MAILSLOT),
    TBL_ENTRY(PIPE),
};
#undef TBL_ENTRY
size_t const resIconTypeMappingSize = sizeof(resIconTypeMapping)/sizeof(resIconTypeMapping[0]);
#endif // RC_INVOKED

#endif // __OBJTYPES_H_VER__
