//https://www.cnblogs.com/caishunzhe/p/12833859.html

#include <iostream>
#include<windows.h>
#include<assert.h>

#include"nt.h"
#include"ntfs.h"

using namespace std;

//x86有调用约定方面的各种各样的问题
#pragma comment(lib,"ntdll")


WCHAR Volume[] = L"\\\\.\\C:";
char mybuffer[0x8000];

LARGE_INTEGER MftStart;
ULONG ClusterSize;
ULONG FrsSize;


VOID
FindAttributeInFileRecord(
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord,
    IN ATTRIBUTE_TYPE_CODE TypeCode,
    IN PATTRIBUTE_RECORD_HEADER PreviousAttribute OPTIONAL,
    OUT PATTRIBUTE_RECORD_HEADER* Attribute
)
// Attribute set to NULL if not found.
{
    PATTRIBUTE_RECORD_HEADER attr;

    *Attribute = NULL;

    if (FileRecord->Pad0[0] != 'F' ||
        FileRecord->Pad0[1] != 'I' ||
        FileRecord->Pad0[2] != 'L' ||
        FileRecord->Pad0[3] != 'E') {

            printf("\nBad MFT record %c%c%c%c",
                FileRecord->Pad0[0],
                FileRecord->Pad0[1],
                FileRecord->Pad0[2],
                FileRecord->Pad0[3]);

        //
        //  This isn't a good file record, but that doesn't make this a corrupt volume.
        //  It's possible that this file record has never been used.  Since we don't look
        //  at the MFT bitmap, we don't know if this was expected to be a valid filerecord.
        //  The output Attribute is set to NULL already, so we can exit now.
        //

        return;
    }

    if (0 == (FileRecord->Flags & FILE_RECORD_SEGMENT_IN_USE)) {

        //
        //  File record not in use, skip it.
        //

        return;
    }

    if (NULL == PreviousAttribute) {

        attr = (PATTRIBUTE_RECORD_HEADER)((PUCHAR)FileRecord + FileRecord->FirstAttributeOffset);

    }
    else {

        attr = (PATTRIBUTE_RECORD_HEADER)((PUCHAR)PreviousAttribute + PreviousAttribute->RecordLength);

        if (((PUCHAR)attr - (PUCHAR)FileRecord) > (LONG)FrsSize) {

            assert(FALSE);
            return;
        }
    }

    while (attr->TypeCode < TypeCode &&
        attr->TypeCode != $END) {

        assert(attr->RecordLength < FrsSize);

        attr = (PATTRIBUTE_RECORD_HEADER)((PUCHAR)attr + attr->RecordLength);

        //
        //  BUGBUG keitha 4/20/00 need to handle attribute list case someday...
        //  It's relativley rare that an MFT gets so fragmented it needs an 
        //  attribute list.  Certainly rare enough to skip it for now in a 
        //  piece of test code.
        //
    }

    if (attr->TypeCode == TypeCode) {

        *Attribute = attr;
    }

    return;
}


int main()
{
    HANDLE volumeHandle;
    DWORD WStatus;
    LARGE_INTEGER byteOffset;
    NTSTATUS ReadStatus;
    PPACKED_BOOT_SECTOR bootSector;
    BIOS_PARAMETER_BLOCK bpb;
    ULONG ClusterSize;
    IO_STATUS_BLOCK IoStatusBlock;
    LONGLONG mftBytesRead;
    PATTRIBUTE_RECORD_HEADER attr;
    ULONG mftRecords;

    volumeHandle = CreateFileW(Volume,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (volumeHandle == INVALID_HANDLE_VALUE) {

        WStatus = GetLastError();
        printf("Unable to open %ws volume\n", &Volume);
        printf("Error from CreateFile %x\n", WStatus);
        return WStatus;
    }


    byteOffset.QuadPart = 0;
    ReadStatus = NtReadFile(volumeHandle,
        NULL,            //  Event
        NULL,            //  ApcRoutine
        NULL,            //  ApcContext
        &IoStatusBlock,
        mybuffer,
        0x200, //512个字节 一个扇区
        &byteOffset,    //  ByteOffset
        NULL);         //  Key

    if (0 != ReadStatus) {

        printf("\nMFT record 0 read failed with status %x", ReadStatus);
        goto exit;
    }

    bootSector = (PPACKED_BOOT_SECTOR)mybuffer;


    if (bootSector->Oem[0] != 'N' ||
        bootSector->Oem[1] != 'T' ||
        bootSector->Oem[2] != 'F' ||
        bootSector->Oem[3] != 'S') {

        printf("\nNot an NTFS volume");
        goto exit;
    }


    NtfsUnpackBios(&bpb, &bootSector->PackedBpb);

    //
    //https://docs.microsoft.com/en-us/windows-server/storage/file-server/ntfs-overview
    //default 4K
    //
    ClusterSize = bpb.BytesPerSector * bpb.SectorsPerCluster;

    //Mft大小一般是1KB
    if (bootSector->ClustersPerFileRecordSegment < 0) {

        FrsSize = 1 << (-1 * bootSector->ClustersPerFileRecordSegment);

    }
    else {

        FrsSize = bootSector->ClustersPerFileRecordSegment * ClusterSize;
    }

    //集群数*集群大小=Mft起始位置
    //0xc0000000/512=0x600000(扇区起始地址数)
    MftStart.QuadPart = ClusterSize * bootSector->MftStartLcn;

    mftBytesRead = 0;

    ReadStatus = NtReadFile(volumeHandle,
        NULL,            //  Event
        NULL,            //  ApcRoutine
        NULL,            //  ApcContext
        &IoStatusBlock,
        mybuffer,
        FrsSize,
        &MftStart,      //  ByteOffset
        NULL);         //  Key

    if (0 != ReadStatus) {

        printf("\nMFT record 0 read failed with status %x", ReadStatus);
        goto exit;
    }

    mftBytesRead += IoStatusBlock.Information;

    FindAttributeInFileRecord((PFILE_RECORD_SEGMENT_HEADER)mybuffer,
        $DATA,
        NULL,
        &attr);

    if (NULL == attr) {

        printf("\nMFT record 0 has no $DATA attribute");
        goto exit;
    }

    if (attr->FormCode == RESIDENT_FORM) {

        printf("\nVolume has very few files, use dir /s");
        goto exit;
    }
    
    assert(attr->Form.Nonresident.FileSize <= ULONG_MAX);
    mftRecords = (ULONG)(attr->Form.Nonresident.FileSize / FrsSize);


















exit:
    if (volumeHandle != NULL) {

        CloseHandle(volumeHandle);
    }

    return 0;
}

