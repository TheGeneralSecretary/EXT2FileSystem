#pragma once

#include <cstdint>

#define EXT2_SIGNATURE 0xEF53
#define EXT2_SUPERBLOCK_OFFSET 1024
#define EXT2_SUPERBLOCK_SIZE 1024

namespace EXT2FS
{
	enum class FileSystemState
	{
		Clean,
		Error
	};

	enum class ErrorHandlingMethod
	{
		Ignore,
		RemountReadOnly,
		KernelPanic
	};

	enum class OperatingSystemID
	{
		Linux,
		GNUHurd,
		Masix,
		FreeBSD,
		Other
	};

	enum class OptionalFeatureFlags
	{
		PreallocatedBlocks = 0x0001,
		AFSServerInode = 0x0002,
		FileSystemJournal = 0x0004,
		InodeExtendedAttrib = 0x0008,
		FileSystemResizable = 0x0010,
		DirHasIndex = 0x0020
	};

	enum class RequiredFeatureFlags
	{
		Compression = 0x0001,
		DirTypeField = 0x0002,
		FileSystemReplayJournal = 0x0004,
		FileSystemJournalDevice = 0x0008,
	};

	enum class ReadOnlyFeatureFlags
	{
		Sparse = 0x0001,
		FileSize64 = 0x0002,
		DirBinaryTree = 0x0004,
	};

#pragma pack(push, 1)
	struct EXT2SuperBlock
	{
		uint32_t InodeCount;
		uint32_t BlockCount;
		uint32_t ReservedBlockCount;
		uint32_t FreeBlockCount;
		uint32_t FreeInodeCount;
		uint32_t FirstDataBlock;
		uint32_t BlockSize;
		uint32_t FragmentSize;
		uint32_t BlockGroupBlockCount;
		uint32_t BlockGroupFragmentCount;
		uint32_t BlockGroupInodeCount;
		uint32_t LastMountTime;
		uint32_t LastWriteTime;
		uint16_t MountCountSinceFSCK;
		uint16_t MountLimitBeforeFSCK;
		uint16_t Signature;
		uint16_t FileSystemState;
		uint16_t ErrorHandler;
		uint16_t MinorVersion;
		uint32_t LaskFSCKTime;
		uint32_t FSCKInterval;
		uint32_t OSID;
		uint32_t MajorVersion;
		uint16_t UserID;
		uint16_t GroupID;
		/* Extended Superblock Fields - MAJOR >= 1 */
		uint32_t FirstInode;
		uint16_t InodeSize;
		uint16_t BlockGroup;
		uint32_t OptionalFeatures;
		uint32_t RequiredFeatues;
		uint32_t UnsupportedFeatures;
		uint8_t FileSystemID[16];
		uint8_t VolumeName[16];
		uint8_t PathVolumeLastMounted[64];
		uint32_t CompressionAlgorithm;
		uint8_t PreallocateBlockCountFile;
		uint8_t PreallocateBlockCountDir;
		uint8_t _Unused1[2];
		uint8_t JournalID[16];
		uint32_t JournalInode;
		uint32_t JournalDevice;
		uint32_t OrphanInodeListHead;
		uint8_t _Unused2[788];
	};
#pragma pack(pop)

#pragma pack(push, 1)
	struct EXT2BlockGroupDescriptor
	{
		uint32_t BlockBitmap;
		uint32_t InodeBitmap;
		uint32_t InodeTable;
		uint16_t FreeBlockCount;
		uint16_t FreeInodeCount;
		uint16_t DirCount;
		uint8_t _unused[14];
	};
#pragma pack(pop)
}
