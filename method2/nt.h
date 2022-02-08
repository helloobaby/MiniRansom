#include<windows.h>

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




}