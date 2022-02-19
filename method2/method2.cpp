/*

这个项目的目的是利用ntfs 的 mft 来遍历C盘的文件

vcn = virtual cluster number
lcn = logical cluster number 

*/

//https://www.cnblogs.com/caishunzhe/p/12833859.html
//https://zh.wikipedia.org/wiki/NTFS
//https://www.sciencedirect.com/topics/computer-science/starting-cluster
//https://www.writebug.com/explore/article/86

#include <iostream>
#include<windows.h>
#include<assert.h>

#include"nt.h"
#include"ntfs.h"

using namespace std;

//x86有调用约定方面的各种各样的问题
#pragma comment(lib,"ntdll")

#define VOLUME_PATH  L"\\\\.\\C:"
#define FULL_PATH    L"\\??\\C:\\symbols"
WCHAR Volume[] = VOLUME_PATH;
char mybuffer[0x8000];

LARGE_INTEGER MftStart;
ULONG ClusterSize;
ULONG FrsSize;

#define Add2Ptr(P,I) ((PVOID)((PUCHAR)(P) + (I)))
#define PtrOffset(B,O) ((ULONG)((ULONG_PTR)(O) - (ULONG_PTR)(B)))



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

bool _is_vaild_mtf_entry(PFILE_RECORD_SEGMENT_HEADER mft_header)
{
    if (mft_header->Pad0[0] != 'F' ||
        mft_header->Pad0[1] != 'I' ||
        mft_header->Pad0[2] != 'L' ||
        mft_header->Pad0[3] != 'E') {   
        
        assert(0);

        return false;
    }

    if (0 == (mft_header->Flags & FILE_RECORD_SEGMENT_IN_USE))
        return false;




    return true;



}

void ParseAttrStandardInfo(const PATTRIBUTE_RECORD_HEADER& attr)
{
    printf("$STANDARD_INFORMATION\n");

    PSTANDARD_INFORMATION t = nullptr;


    if(attr->FormCode == RESIDENT_FORM) //Resident部分+头部 = 0x18
        t = (PSTANDARD_INFORMATION)((uint8_t*) attr + 0x18);

    assert(t); //非常驻

    printf("    Identificator %x\n", attr->Instance);

    //Update Sequence Number
    printf("    Usn: %llx\n",t->Usn);

}

void ParseAttrFileName(const PATTRIBUTE_RECORD_HEADER& attr)
{
    printf("$FILE_NAME\n");

    PFILE_NAME t = nullptr;

    if (attr->FormCode == RESIDENT_FORM)
        t = (PFILE_NAME)((uint8_t*)attr + 0x18);

    assert(t);

    printf("    Identificator %x\n", attr->Instance);
    printf("    Name: %ws\n", t->FileName);
}

void ParseAttrData(const PATTRIBUTE_RECORD_HEADER& attr)
{
    PUCHAR NextMappingPairs;
    UCHAR VLength;
    UCHAR LLength;

    uint64_t qwTmp = 0;

    printf("$DATA\n");

    /*
    * 2022.2.19
    * 数据长度多了就非常驻，太少的就常驻
    * 
    */
    //DATA一般都是非常驻的
    //assert(attr->FormCode == NONRESIDENT_FORM);

    printf("    Identificator %x\n", attr->Instance);

    printf("    Start VCN %llx\n", attr->Form.Nonresident.LowestVcn);
    printf("    Last VCN %llx\n", attr->Form.Nonresident.HighestVcn);

    //MappingPairsOffset
    printf("    Offset to Run list %x\n", attr->Form.Nonresident.MappingPairsOffset);
    printf("    Allocated size %llx\n", attr->Form.Nonresident.AllocatedLength);
    printf("    CompressionUnit %d\n", attr->Form.Nonresident.CompressionUnit);

    
    NextMappingPairs = (PUCHAR)Add2Ptr(attr, attr->Form.Nonresident.MappingPairsOffset);
    
    do
    {
        LLength = *NextMappingPairs >> 4; //起始簇号占用几字节
        VLength = *NextMappingPairs & 0x0f; //低位  大小占用字节数

        memcpy_s(&qwTmp, sizeof(qwTmp), NextMappingPairs+1, VLength);

        printf("\tsize: %llx", qwTmp);

        qwTmp = 0;
        memcpy_s(&qwTmp, sizeof(qwTmp), NextMappingPairs + 1 + VLength, LLength);

        printf("\tclusters starting at %llx\n", qwTmp);

        qwTmp = 0;
        NextMappingPairs = NextMappingPairs + LLength + VLength + 1 ;

    }
    while (LLength);



}


void ParseMftEntry(const PFILE_RECORD_SEGMENT_HEADER &mft_header)
{

    if (!_is_vaild_mtf_entry(mft_header))
    {
        return;
    }


    PATTRIBUTE_RECORD_HEADER attr;

    attr = (PATTRIBUTE_RECORD_HEADER)((PUCHAR)mft_header + mft_header->FirstAttributeOffset);

    while (attr->TypeCode != $END) {

        assert(attr->RecordLength < FrsSize);

        switch (attr->TypeCode)
        {
        case $STANDARD_INFORMATION :
            ParseAttrStandardInfo(attr);
            break;
        case $FILE_NAME:
            ParseAttrFileName(attr);
            break;
        case $DATA:
            ParseAttrData(attr);
            break;
        case $BITMAP:
            break;
        case $OBJECT_ID:
            break;
        case $VOLUME_NAME:
            break;
        case $VOLUME_INFORMATION:
            break;
        case $SECURITY_DESCRIPTOR:
            break;
        default:
            assert(0);
        }












        attr = (PATTRIBUTE_RECORD_HEADER)((PUCHAR)attr + attr->RecordLength);
    }




}


int main()
{
    HANDLE volumeHandle;
    DWORD WStatus;
    LARGE_INTEGER byteOffset;
    NTSTATUS ReadStatus;
    PPACKED_BOOT_SECTOR bootSector;
    BIOS_PARAMETER_BLOCK bpb;
    IO_STATUS_BLOCK IoStatusBlock;
    LONGLONG mftBytesRead;
    PATTRIBUTE_RECORD_HEADER attr;
    ULONG recordIndex;
    ULONG mftRecords;
    void* allMftBuffer;

    volumeHandle = CreateFileW(L"\\\\.\\C:",
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

        if (volumeHandle != INVALID_HANDLE_VALUE) {

            CloseHandle(volumeHandle);
        }
    }

    bootSector = (PPACKED_BOOT_SECTOR)mybuffer;


    if (bootSector->Oem[0] != 'N' ||
        bootSector->Oem[1] != 'T' ||
        bootSector->Oem[2] != 'F' ||
        bootSector->Oem[3] != 'S') {

        printf("\nNot an NTFS volume");

        if (volumeHandle != INVALID_HANDLE_VALUE) {

            CloseHandle(volumeHandle);
        }
    }


    NtfsUnpackBios(&bpb, &bootSector->PackedBpb);

    //
    //https://docs.microsoft.com/en-us/windows-server/storage/file-server/ntfs-overview
    //default 4K
    //
    ClusterSize = bpb.BytesPerSector * bpb.SectorsPerCluster;

    //Mft大小(FrsSize)一般是1KB
    if (bootSector->ClustersPerFileRecordSegment < 0) {

        FrsSize = 1 << (-1 * bootSector->ClustersPerFileRecordSegment);

    }
    else {

        FrsSize = bootSector->ClustersPerFileRecordSegment * ClusterSize;
    }

    //集群数*集群大小=Mft起始位置
    //0xc0000000/512=0x600000(扇区起始地址数)
    MftStart.QuadPart = ClusterSize * bootSector->MftStartLcn;

    //MftStart.QuadPart += 0xA * 512;  //dir mft的地址

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
        
        if (volumeHandle != INVALID_HANDLE_VALUE) {

            CloseHandle(volumeHandle);
        }
    }

    mftBytesRead += IoStatusBlock.Information;
    
    FindAttributeInFileRecord((PFILE_RECORD_SEGMENT_HEADER)mybuffer,
        $DATA,
        NULL,
        &attr);

    //2022.2.13 0x0007d600
    mftRecords = (ULONG)(attr->Form.Nonresident.FileSize / FrsSize);

    allMftBuffer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, mftRecords * 1024);

    if (!allMftBuffer) {
        
        if (volumeHandle != INVALID_HANDLE_VALUE) {

            CloseHandle(volumeHandle);
        }

        return 0;
    }
    ReadStatus = NtReadFile(volumeHandle,
        NULL,            //  Event
        NULL,            //  ApcRoutine
        NULL,            //  ApcContext
        &IoStatusBlock,
        allMftBuffer,
        mftRecords * 1024,
        &MftStart,      //  ByteOffset   0xC0000000/512=0x600000(扇区)
        NULL);         //  Key

    if (ReadStatus < 0)
    {

        if (volumeHandle != INVALID_HANDLE_VALUE)
            CloseHandle(volumeHandle);

        return 0;
    }

    for (recordIndex = 0; recordIndex <= mftRecords; recordIndex++) {

        ParseMftEntry((PFILE_RECORD_SEGMENT_HEADER)(uint8_t*)allMftBuffer);




        allMftBuffer = ((uint8_t*)allMftBuffer + 1024);








    }











    HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY, allMftBuffer);

    return 0;
}

