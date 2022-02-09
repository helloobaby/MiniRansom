#include<windows.h>

#define FILE_DIRECTORY_FILE                     0x00000001
#define FILE_WRITE_THROUGH                      0x00000002
#define FILE_SEQUENTIAL_ONLY                    0x00000004
#define FILE_NO_INTERMEDIATE_BUFFERING          0x00000008

#define FILE_SYNCHRONOUS_IO_ALERT               0x00000010
#define FILE_SYNCHRONOUS_IO_NONALERT            0x00000020
#define FILE_NON_DIRECTORY_FILE                 0x00000040
#define FILE_CREATE_TREE_CONNECTION             0x00000080

#define FILE_COMPLETE_IF_OPLOCKED               0x00000100
#define FILE_NO_EA_KNOWLEDGE                    0x00000200
#define FILE_OPEN_REMOTE_INSTANCE               0x00000400
#define FILE_RANDOM_ACCESS                      0x00000800

#define FILE_DELETE_ON_CLOSE                    0x00001000
#define FILE_OPEN_BY_FILE_ID                    0x00002000
#define FILE_OPEN_FOR_BACKUP_INTENT             0x00004000
#define FILE_NO_COMPRESSION                     0x0000800

#define FILE_SUPERSEDE                  0x00000000
#define FILE_OPEN                       0x00000001
#define FILE_CREATE                     0x00000002
#define FILE_OPEN_IF                    0x00000003
#define FILE_OVERWRITE                  0x00000004
#define FILE_OVERWRITE_IF               0x00000005
#define FILE_MAXIMUM_DISPOSITION        0x00000005

#define OBJ_INHERIT             0x00000002L
#define OBJ_PERMANENT           0x00000010L
#define OBJ_EXCLUSIVE           0x00000020L
#define OBJ_CASE_INSENSITIVE    0x00000040L
#define OBJ_OPENIF              0x00000080L
#define OBJ_OPENLINK            0x00000100L
#define OBJ_VALID_ATTRIBUTES    0x000001F2L

typedef __success(return >= 0) LONG NTSTATUS;
#define STATUS_SUCCESS   ((NTSTATUS)0x00000000L)
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

#define InitializeObjectAttributes( p, n, a, r, s ) { \
    (p)->Length = sizeof( OBJECT_ATTRIBUTES );          \
    (p)->RootDirectory = r;                             \
    (p)->Attributes = a;                                \
    (p)->ObjectName = n;                                \
    (p)->SecurityDescriptor = s;                        \
    (p)->SecurityQualityOfService = NULL;               \
    }

typedef enum _FILE_INFORMATION_CLASS {
    FileDirectoryInformation = 1,
    FileFullDirectoryInformation,                   // 2
    FileBothDirectoryInformation,                   // 3
    FileBasicInformation,                           // 4
    FileStandardInformation,                        // 5
    FileInternalInformation,                        // 6
    FileEaInformation,                              // 7
    FileAccessInformation,                          // 8
    FileNameInformation,                            // 9
    FileRenameInformation,                          // 10
    FileLinkInformation,                            // 11
    FileNamesInformation,                           // 12
    FileDispositionInformation,                     // 13
    FilePositionInformation,                        // 14
    FileFullEaInformation,                          // 15
    FileModeInformation,                            // 16
    FileAlignmentInformation,                       // 17
    FileAllInformation,                             // 18
    FileAllocationInformation,                      // 19
    FileEndOfFileInformation,                       // 20
    FileAlternateNameInformation,                   // 21
    FileStreamInformation,                          // 22
    FilePipeInformation,                            // 23
    FilePipeLocalInformation,                       // 24
    FilePipeRemoteInformation,                      // 25
    FileMailslotQueryInformation,                   // 26
    FileMailslotSetInformation,                     // 27
    FileCompressionInformation,                     // 28
    FileObjectIdInformation,                        // 29
    FileCompletionInformation,                      // 30
    FileMoveClusterInformation,                     // 31
    FileQuotaInformation,                           // 32
    FileReparsePointInformation,                    // 33
    FileNetworkOpenInformation,                     // 34
    FileAttributeTagInformation,                    // 35
    FileTrackingInformation,                        // 36
    FileIdBothDirectoryInformation,                 // 37
    FileIdFullDirectoryInformation,                 // 38
    FileValidDataLengthInformation,                 // 39
    FileShortNameInformation,                       // 40
    FileIoCompletionNotificationInformation,        // 41
    FileIoStatusBlockRangeInformation,              // 42
    FileIoPriorityHintInformation,                  // 43
    FileSfioReserveInformation,                     // 44
    FileSfioVolumeInformation,                      // 45
    FileHardLinkInformation,                        // 46
    FileProcessIdsUsingFileInformation,             // 47
    FileNormalizedNameInformation,                  // 48
    FileNetworkPhysicalNameInformation,             // 49
    FileIdGlobalTxDirectoryInformation,             // 50
    FileIsRemoteDeviceInformation,                  // 51
    FileUnusedInformation,                          // 52
    FileNumaNodeInformation,                        // 53
    FileStandardLinkInformation,                    // 54
    FileRemoteProtocolInformation,                  // 55

        //
        //  These are special versions of these operations (defined earlier)
        //  which can be used by kernel mode drivers only to bypass security
        //  access checks for Rename and HardLink operations.  These operations
        //  are only recognized by the IOManager, a file system should never
        //  receive these.
        //

        FileRenameInformationBypassAccessCheck,         // 56
        FileLinkInformationBypassAccessCheck,           // 57

            //
            // End of special information classes reserved for IOManager.
            //

            FileVolumeNameInformation,                      // 58
            FileIdInformation,                              // 59
            FileIdExtdDirectoryInformation,                 // 60
            FileReplaceCompletionInformation,               // 61
            FileHardLinkFullIdInformation,                  // 62
            FileIdExtdBothDirectoryInformation,             // 63
            FileDispositionInformationEx,                   // 64
            FileRenameInformationEx,                        // 65
            FileRenameInformationExBypassAccessCheck,       // 66
            FileDesiredStorageClassInformation,             // 67
            FileStatInformation,                            // 68
            FileMemoryPartitionInformation,                 // 69
            FileStatLxInformation,                          // 70
            FileCaseSensitiveInformation,                   // 71
            FileLinkInformationEx,                          // 72
            FileLinkInformationExBypassAccessCheck,         // 73
            FileStorageReserveIdInformation,                // 74
            FileCaseSensitiveInformationForceAccessCheck,   // 75
            FileKnownFolderInformation,   // 76

            FileMaximumInformation
} FILE_INFORMATION_CLASS, * PFILE_INFORMATION_CLASS;

typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        PVOID    Pointer;
    };
    ULONG_PTR Information;
} IO_STATUS_BLOCK, * PIO_STATUS_BLOCK;

typedef
VOID
(NTAPI* PIO_APC_ROUTINE) (
    _In_ PVOID ApcContext,
    _In_ PIO_STATUS_BLOCK IoStatusBlock,
    _In_ ULONG Reserved
    );


typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, * PUNICODE_STRING;

typedef struct _OBJECT_ATTRIBUTES {
    ULONG           Length;
    HANDLE          RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG           Attributes;
    PVOID           SecurityDescriptor;
    PVOID           SecurityQualityOfService;
} OBJECT_ATTRIBUTES,*POBJECT_ATTRIBUTES;

typedef struct _FILE_NAME_INFORMATION {
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_NAME_INFORMATION, * PFILE_NAME_INFORMATION;


extern "C"
{

    NTSTATUS NtReadFile(
        _In_     HANDLE           FileHandle,
        _In_opt_ HANDLE           Event,
        _In_opt_ PIO_APC_ROUTINE  ApcRoutine,
        _In_opt_ PVOID            ApcContext,
        _Out_    PIO_STATUS_BLOCK IoStatusBlock,
        _Out_    PVOID            Buffer,
        _In_     ULONG            Length,
        _In_opt_ PLARGE_INTEGER   ByteOffset,
        _In_opt_ PULONG           Key
    );



    NTSYSAPI VOID RtlInitUnicodeString(
        PUNICODE_STRING         DestinationString,
        PCWSTR SourceString
    );

    __kernel_entry NTSTATUS NtCreateFile(
        PHANDLE            FileHandle,
        ACCESS_MASK        DesiredAccess,
        POBJECT_ATTRIBUTES ObjectAttributes,
        PIO_STATUS_BLOCK   IoStatusBlock,
        PLARGE_INTEGER     AllocationSize,
        ULONG              FileAttributes,
        ULONG              ShareAccess,
        ULONG              CreateDisposition,
        ULONG              CreateOptions,
        PVOID              EaBuffer,
        ULONG              EaLength
    );

    __kernel_entry  NTSTATUS NtQueryInformationFile(
        HANDLE                 FileHandle,
        PIO_STATUS_BLOCK       IoStatusBlock,
        PVOID                  FileInformation,
        ULONG                  Length,
        FILE_INFORMATION_CLASS FileInformationClass
    );

    __kernel_entry NTSTATUS NtClose(
        HANDLE Handle
    );




}