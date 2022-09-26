#pragma once
#include <Windows.h>

//
// Header file for the VX Logging Library.
//

#define VXLL_VERSION 1

//
// Structure, Union & Enumeration definitions
//

typedef enum _VXLSTATUS {
	VXL_SUCCESS,
	VXL_FAILURE,
	VXL_OUT_OF_MEMORY,
	VXL_NOT_IMPLEMENTED,
	VXL_INSUFFICIENT_CREDENTIALS,
	VXL_INVALID_PARAMETER,
	VXL_INVALID_PARAMETER_LENGTH,
	VXL_INVALID_PARAMETER_COMBINATION,
	VXL_INSUFFICIENT_BUFFER,
	VXL_FILE_ALREADY_OPENED,
	VXL_FILE_ALREADY_EXISTS,
	VXL_FILE_IO_ERROR,
	VXL_FILE_SYNCHRONIZATION_ERROR,
	VXL_FILE_WRONG_MODE,
	VXL_FILE_NOT_FOUND,
	VXL_FILE_CORRUPT,
	VXL_FILE_INVALID,
	VXL_FILE_CANNOT_OPEN,
	VXL_FILE_MISMATCHED_SOURCE_APPLICATION,
	VXL_FILE_SOURCE_COMPONENT_NOT_FOUND,
	VXL_FILE_SOURCE_COMPONENT_LIMIT_EXCEEDED,
	VXL_ENTRY_NOT_FOUND,
	VXL_ENTRY_OUT_OF_RANGE,
	VXL_MAXIMUM_ERROR_CODE
} VXLSTATUS;

typedef enum _VXLLOGINFOCLASS {
	LogLibraryVersion,
	LogNumberOfCriticalEvents,
	LogNumberOfErrorEvents,
	LogNumberOfWarningEvents,
	LogNumberOfInformationEvents,
	LogNumberOfDetailEvents,
	LogNumberOfDebugEvents,
	LogNumberOfEntries,
	LogSourceApplication,
	LogSourceComponents
} VXLLOGINFOCLASS;

typedef enum _VXLSEVERITY {
	LogSeverityInvalidValue = -1,
	LogSeverityCritical,
	LogSeverityError,
	LogSeverityWarning,
	LogSeverityInformation,
	LogSeverityDetail,
	LogSeverityDebug,
	LogSeverityMaximumValue
} VXLSEVERITY;

typedef struct _VXLLOGENTRY {
	PWSTR		Text;
	PCWSTR		SourceComponent;
	PWSTR		SourceFile;
	PWSTR		SourceFunction;
	ULONG		SourceLine;
	VXLSEVERITY	Severity;
	SYSTEMTIME	Time;
} VXLLOGENTRY, *PVXLLOGENTRY, **PPVXLLOGENTRY, *CONST PCVXLLOGENTRY, **CONST PPCVXLLOGENTRY;

typedef struct _VXLLOGFILEHEADER {
	CHAR		Magic[4];
	ULONG		Version;
	ULONG		EventSeverityTypeCount[LogSeverityMaximumValue];
	WCHAR		SourceApplication[64];
	WCHAR		SourceComponents[64][16];
	BOOLEAN		Dirty;
} VXLLOGFILEHEADER, *PVXLLOGFILEHEADER, **PPVXLLOGFILEHEADER, *CONST PCVXLLOGFILEHEADER, **CONST PPCVXLLOGFILEHEADER;

typedef struct _VXLLOGFILEENTRY {
	union {
		FILETIME	Time;
		ULONGLONG	Time64;
	};
	VXLSEVERITY	Severity;
	ULONG		SourceLine;
	USHORT		SourceComponentIndex;
	USHORT		SourceFileLength;
	USHORT		SourceFunctionLength;
	USHORT		TextLength;
} VXLLOGFILEENTRY, *PVXLLOGFILEENTRY, **PPVXLLOGFILEENTY, *CONST PCVXLLOGFILEENTRY, **CONST PPCVXLLOGFILEENTRY;

// index cache (EntryIndexToFileOffset and SeverityIndexToEntryIndex) makes
// reading and sorting the log file faster. Without it, writing the log file
// is very fast but read and export performance is unacceptably bad.
typedef struct _VXLCONTEXT {
	CRITICAL_SECTION	Lock;
	HANDLE				FileHandle;
	ULONG				OpenFlags;
	USHORT				SourceComponentIndex;
	VXLLOGFILEHEADER	Header;
	VXLLOGENTRY			LastCriticalEntry;
	PLARGE_INTEGER		EntryIndexToFileOffset; // only populated when file is opened for READ ONLY, otherwise NULL
} VXLCONTEXT, *PVXLCONTEXT, **PPVXLCONTEXT, *CONST PCVXLCONTEXT, **CONST PPCVXLCONTEXT;

//
// Basic Type definitions
//

typedef PVXLCONTEXT VXLHANDLE, *PVXLHANDLE, *CONST PCVXLHANDLE;

//
// Macros
//

#define VXLAPI __cdecl
#define VXL_SUCCEEDED(Status) ((Status) == VXL_SUCCESS)
#define VXL_FAILED(Status) (!VXL_SUCCEEDED(Status))

#define VXL_OPEN_WRITE_ONLY										1
#define VXL_OPEN_READ_ONLY										2
#define VXL_OPEN_CREATE_IF_NOT_EXISTS							4
#define VXL_OPEN_APPEND_IF_EXISTS								8
#define VXL_OPEN_OVERWRITE_IF_EXISTS							16
#define VXL_OPEN_IGNORE_CORRUPTION								32

//
// Function Declarations
//

VXLSTATUS VXLAPI VxlOpenLogFile(
	IN		PCWSTR			FileName,
	OUT		PVXLHANDLE		LogHandle,
	IN		PCWSTR			SourceApplication,
	IN		PCWSTR			SourceComponent);

VXLSTATUS VXLAPI VxlOpenLogFileReadOnly(
	IN		PCWSTR			FileName,
	OUT		PVXLHANDLE		LogHandle);

VXLSTATUS VXLAPI VxlOpenLogFileEx(
	IN		PCWSTR			FileName,
	OUT		PVXLHANDLE		LogHandle,
	IN		PCWSTR			SourceApplication,
	IN		PCWSTR			SourceComponent,
	IN		ULONG			Flags);

VXLSTATUS VXLAPI VxlCloseLogFile(
	IN OUT	PVXLHANDLE		LogHandle);

VXLSTATUS VXLAPI VxlQueryLogInformation(
	IN		VXLHANDLE		LogHandle,
	IN		VXLLOGINFOCLASS	InformationClass,
	OUT		PVOID			Buffer,
	IN OUT	PULONG			BufferSize);

#ifndef __L
#  define __L(s) L ## s
#endif

#ifndef _L
#  define _L(s) __L(s)
#endif

#define VxlLog(LogHandle, Severity, ...) VxlLogEx(LogHandle, _L(__FILE__), __LINE__, _L(__FUNCTION__), Severity, __VA_ARGS__)

VXLSTATUS VXLAPI VxlLogEx(
	IN		VXLHANDLE		LogHandle,
	IN		PCWSTR			SourceFile OPTIONAL,
	IN		ULONG			SourceLine,
	IN		PCWSTR			SourceFunction OPTIONAL,
	IN		VXLSEVERITY		Severity,
	IN		PCWSTR			Format, ...);

VXLSTATUS VXLAPI VxlReadLastCriticalEntry(
	IN		VXLHANDLE		LogHandle,
	OUT		PPCVXLLOGENTRY	Entry);

VXLSTATUS VXLAPI VxlReadLogEntry(
	IN		VXLHANDLE		LogHandle,
	IN		ULONG			EntryIndex,
	OUT		PPVXLLOGENTRY	Entry);

VXLSTATUS VXLAPI VxlConvertLogEntryToText(
	IN		PVXLLOGENTRY	Entry,
	OUT		PWSTR			Text OPTIONAL,
	IN OUT	PULONG			TextLength,
	IN		BOOLEAN			LongForm);

VXLSTATUS VXLAPI VxlFreeLogEntry(
	IN OUT	PPVXLLOGENTRY	Entry);

VXLSTATUS VXLAPI VxlExportLogToText(
	IN		VXLHANDLE		LogHandle,
	IN		PCWSTR			TextFilePath,
	IN		BOOLEAN			LongForm);

PCWSTR VXLAPI VxlErrorLookup(
	IN		VXLSTATUS		Status);

PCWSTR VXLAPI VxlSeverityLookup(
	IN		VXLSEVERITY		Severity,
	IN		BOOLEAN			LongDescription);