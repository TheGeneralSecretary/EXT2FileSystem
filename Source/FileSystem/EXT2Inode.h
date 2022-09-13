#pragma once

#include <stdint.h>

#define EXT2_MAX_FILENAME 255
#define EXT2_DBP_SIZE 12
#define EXT2_ROOT_INODE 2

namespace EXT2FS
{
	enum class InodeType
	{
		FIFO = 0x1000,
		CharacterDevice = 0x2000,
		Directory = 0x4000,
		BlockDevice = 0x6000,
		RegularFile = 0x8000,
		SymbolicLink = 0xA000,
		UnixSocket = 0xC000
	};

	enum class InodePermission
	{
		OtherExecute = 0x001,
		OtherWrite = 0x002,
		OtherRead = 0x004,
		GroupExecute = 0x008,
		GroupWrite = 0x010,
		GroupRead = 0x020,
		UserExecute = 0x040,
		UserWrite = 0x080,
		UserRead = 0x100,
		StickyBit = 0x200,
		SetGroupID = 0x400,
		SetUserID = 0x800
	};

	enum class InodeFlags
	{
		SecureDeletion = 0x00000001,
		KeepCopyAfterDelete = 0x00000002,
		FileCompression = 0x00000004,
		SynchronousUpdate = 0x00000008,
		ImmutableFile = 0x00000010,
		FileNoIncludeInDump = 0x00000020,
		LastAccessTimeNoUpdate = 0x00000040,
		HashIndexedDir = 0x00010000,
		AFSDir = 0x00020000,
		JournalFileData = 0x00040000,
	};

	enum class DirentType
	{
		Unknown = 0,
		RegularFile = 1,
		Directory = 2,
		CharacterDevice = 3,
		BlockDevice = 4,
		FIFO = 5,
		Socket = 6,
		SymbolicLink = 7
	};

#pragma pack(push, 1)
	struct EXT2Inode
	{
		uint16_t Mode;
		uint16_t UserID;
		uint32_t SizeLow;
		uint32_t LastAccessTime;
		uint32_t LastCreationTime;
		uint32_t LastModificationTime;
		uint32_t DeletionTime;
		uint16_t GroupID;
		uint16_t HardLinkCount;
		uint32_t SectorCount;
		uint32_t Flags;
		uint32_t OSSV1;
		uint32_t DBP[EXT2_DBP_SIZE];
		uint32_t SIBP;
		uint32_t DIBP;
		uint32_t TIBP;
		uint32_t Generation;
		union {
			uint32_t Reserved1;
			uint32_t FileACL;
		};
		union {
			uint32_t Reserved2;
			uint32_t SizeHigh;
			uint32_t DirACL;
		};
		uint32_t FragmentAddr;
		uint8_t OSSV2[12];
	};
#pragma pack(pop)

#pragma pack(push, 1)
	struct EXT2Dirent
	{
		uint32_t Inode;
		uint16_t Size;
		uint8_t NameLength;
		uint8_t Type;
		uint8_t Name[EXT2_MAX_FILENAME];
	};
#pragma pack(pop)
}
