#include "buildpch.h"
#include "FileSystem/EXT2FileSystem.h"
#include "Core/Logger.h"
#include "FileSystem/EXT2PathParser.h"

#include <fstream>

namespace EXT2FS
{
	bool EXT2FileSystem::Init()
	{
		m_CurrentPath = "/";
		return true;
	}

	void EXT2FileSystem::Destroy()
	{
		m_Device = "";
	}

	bool EXT2FileSystem::OpenDevice(const std::string& devpath)
	{
		std::ifstream stream(devpath, std::ios::binary);
		if (stream.fail())
		{
			LOGGER_ERROR("OpenDevice Failed To Open: {}", devpath);
			return false;
		}
		stream.close();
		m_Device = devpath;

		if (!ParseSuperblock())
		{
			LOGGER_ERROR("OpenDevice Failed To ParseSuperblock: {}", devpath);
			return false;
		}

		if (!ParseBlockGroups())
		{
			LOGGER_ERROR("OpenDevice Failed To ParseBlockGroups: {}", devpath);
			return false;
		}

		m_RootInode = GetInode(EXT2_ROOT_INODE);
		if (!m_RootInode)
		{
			LOGGER_ERROR("OpenDevice Failed To GetInode(ROOT_INODE): {}", devpath);
			return false;
		}

		return true;
	}

	void EXT2FileSystem::LS(const std::string& path)
	{
		if (!path.size() && m_CurrentPath == "/")
		{
			DumpInodeNodes(m_RootInode);
			return;
		}

		Ref<EXT2Inode> inodebuf;
		if (path[0] != '/')
			inodebuf = GetInodeFromPath(m_CurrentPath + path);
		else
			inodebuf = GetInodeFromPath(path);

		if (!inodebuf)
		{
			LOGGER_ERROR("Failed to Get: {}", path);
			return;
		}

		DumpInodeNodes(inodebuf);
	}

	void EXT2FileSystem::CD(const std::string& path)
	{
		if (path == m_CurrentPath) return;
		if (path == "/")
		{
			LOGGER_INFO("Switched Directory To: {}", path);
			m_CurrentPath == "/";
			return;
		}

		Ref<EXT2Inode> inodebuf;
		if (path[0] != '/')
			inodebuf = GetInodeFromPath(m_CurrentPath + path);
		else
			inodebuf = GetInodeFromPath(path);

		if (!inodebuf)
		{
			LOGGER_ERROR("No Such Directory: {}", path);
			return;
		}

		if (!(inodebuf->Mode & (int)InodeType::Directory))
		{
			LOGGER_ERROR("Not a Directory: {}", path);
			return;
		}

		LOGGER_INFO("Switched Directory To: {}", path);
		if(path[0] != '/')
			m_CurrentPath = "/" + path;
		else
			m_CurrentPath = path;
	}

	bool EXT2FileSystem::ParseSuperblock()
	{
		std::ifstream stream(m_Device, std::ios::binary);
		if (stream.fail())
		{
			LOGGER_ERROR("EXT2ParseSuperblock Failed To Open: {}", m_Device);
			return false;
		}

		uint8_t* sb = new uint8_t[EXT2_SUPERBLOCK_SIZE];
		memset(sb, 0, EXT2_SUPERBLOCK_SIZE);
		stream.seekg(EXT2_SUPERBLOCK_OFFSET);
		stream.read((char*)sb, EXT2_SUPERBLOCK_SIZE);

		EXT2SuperBlock* superblock = (EXT2SuperBlock*)sb;
		if (superblock->Signature != EXT2_SIGNATURE)
		{
			LOGGER_ERROR("EXT2ParseSuperblock Invalid Signature: {}", superblock->Signature);
			return false;
		}
		superblock->BlockSize = (1024 << superblock->BlockSize);
		superblock->FragmentSize = (1024 << superblock->BlockSize);

		LOGGER_INFO("EXT2 Signature: {} EXT2: Version: {}.{}", superblock->Signature, superblock->MajorVersion, superblock->MinorVersion);

		m_Superblock.reset(superblock);
		return true;
	}

	bool EXT2FileSystem::ParseBlockGroups()
	{
		m_FirstBlockGroup = m_Superblock->FirstDataBlock + 1;
		m_BlockGroupDescriptorCount = m_Superblock->BlockCount / m_Superblock->BlockGroupBlockCount;
		if (!m_BlockGroupDescriptorCount) m_BlockGroupDescriptorCount = 1;

		for (uint32_t i = 0; i < m_BlockGroupDescriptorCount; i++)
		{
			uint32_t index = (i * sizeof(EXT2BlockGroupDescriptor)) / m_Superblock->BlockSize;
			uint32_t offset = (i * sizeof(EXT2BlockGroupDescriptor)) % m_Superblock->BlockSize;

			uint8_t* block = ReadBlock(m_FirstBlockGroup + index);
			m_BlockGroupDescriptors.push_back(*(EXT2BlockGroupDescriptor*)(block + offset));
			delete block;
		}

		return true;
	}

	const EXT2BlockGroupDescriptor& EXT2FileSystem::GetBlockGroup(uint32_t group)
	{
		return m_BlockGroupDescriptors[group];
	}

	Ref<EXT2Inode> EXT2FileSystem::GetInode(uint32_t inode)
	{
		if (inode == 0) return nullptr;

		uint32_t bg = (inode - 1) / m_Superblock->BlockGroupInodeCount;
		auto& bgd = GetBlockGroup(bg);

		uint32_t index = (inode - 1) % m_Superblock->BlockGroupInodeCount;
		uint32_t block = (index * m_Superblock->InodeSize) / m_Superblock->BlockSize;

		index = index % (m_Superblock->BlockSize / m_Superblock->InodeSize);
		uint32_t offset = (index * m_Superblock->InodeSize);

		uint8_t* inodetable = ReadBlock(bgd.InodeTable + block); // OPTIMIZE
		Ref<EXT2Inode> inodebuf = CreateRef<EXT2Inode>(*(EXT2Inode*)(inodetable + offset));
		delete inodetable;
		return inodebuf;
	}

	Ref<EXT2Inode> EXT2FileSystem::GetInodeFromDir(const Ref<EXT2Inode>& inodebuf, const std::string& name)
	{
		if (!(inodebuf->Mode & (int)InodeType::Directory)) return nullptr;

		for (uint32_t i = 0; i < EXT2_DBP_SIZE; i++)
		{
			if (!inodebuf->DBP[i]) continue;

			uint8_t* data = ReadBlock(inodebuf->DBP[i]);
			EXT2Dirent* dir = (EXT2Dirent*)data;

			if (dir->Size == 0 || dir->NameLength == 0 || dir->Inode <= 0 || dir->Inode >= m_Superblock->InodeCount)
			{
				delete data;
				break;
			}

			while (dir->NameLength > 0 && dir->Inode > 0 && dir->Inode < m_Superblock->InodeCount)
			{
				dir->Name[dir->NameLength] = 0;
				if (!strcmp((char*)dir->Name, name.c_str()))
				{
					uint32_t inode = dir->Inode;
					delete data;
					return GetInode(inode);
				}
				dir = (EXT2Dirent*)((uint8_t*)dir + dir->Size);
			}

			delete data;
		}
		return nullptr;
	}

	Ref<EXT2Inode> EXT2FileSystem::GetInodeFromPath(const std::string& path)
	{
		EXT2Path parsedpath = EXT2ParsePath(path);
		if(!parsedpath)
		{
			LOGGER_ERROR("Failed to ParsePath: {}", path);
			return nullptr;
		}

		if (parsedpath.path == "/") return m_RootInode;

		auto node = m_RootInode;
		for (uint32_t i = 0; i < parsedpath.parts.size(); i++)
		{
			node = GetInodeFromDir(node, parsedpath.parts[i]);
			if (!node)
			{
				LOGGER_ERROR("Failed to Get: {} From: {}", parsedpath.parts[i], path);
				return nullptr;
			}

			if (!(node->Mode & (int)InodeType::Directory))
			{
				LOGGER_ERROR("Not a Directory: {}", parsedpath.parts[i], path);
				return nullptr;
			}
		}

		return GetInodeFromDir(node, parsedpath.target);
	}

	uint8_t* EXT2FileSystem::ReadBlock(uint32_t block)
	{
		std::ifstream stream(m_Device, std::ios::binary);
		if (stream.fail())
		{
			std::cerr << "ReadBlock Failed To Open: " + m_Device << std::endl;
			return nullptr;
		}

		uint8_t* data = new uint8_t[m_Superblock->BlockSize];
		memset(data, 0, m_Superblock->BlockSize);
		stream.seekg(block * m_Superblock->BlockSize);
		stream.read((char*)data, m_Superblock->BlockSize);
		return data;
	}

	void EXT2FileSystem::DumpSuperblock()
	{
		printf("\n-----EXT2 DUMP SUPERBLOCK START-----\n");
		printf("InodeCount = %d\n", m_Superblock->InodeCount);
		printf("BlockCount = %d\n", m_Superblock->BlockCount);
		printf("ReservedBlockCount = %d\n", m_Superblock->ReservedBlockCount);
		printf("FreeBlockCount = %d\n", m_Superblock->FreeBlockCount);
		printf("FreeInodeCount = %d\n", m_Superblock->FreeInodeCount);
		printf("FirstDataBlock = %d\n", m_Superblock->FirstDataBlock);
		printf("BlockSize = %d\n", m_Superblock->BlockSize);
		printf("FragmentSize = %d\n", m_Superblock->FragmentSize);
		printf("BlockGroupBlockCount = %d\n", m_Superblock->BlockGroupBlockCount);
		printf("BlockGroupFragmentCount = %d\n", m_Superblock->BlockGroupFragmentCount);
		printf("BlockGroupInodeCount = %d\n", m_Superblock->BlockGroupInodeCount);
		printf("LastMountTime = %d\n", m_Superblock->LastMountTime);
		printf("LastWriteTime = %d\n", m_Superblock->LastWriteTime);
		printf("MountCountSinceFSCK = %d\n", m_Superblock->MountCountSinceFSCK);
		printf("MountLimitBeforeFSCK = %d\n", m_Superblock->MountLimitBeforeFSCK);
		printf("Signature = %x\n", m_Superblock->Signature);
		printf("FileSystemState = %d\n", m_Superblock->FileSystemState);
		printf("ErrorHandler = %d\n", m_Superblock->ErrorHandler);
		printf("MinorVersion = %d\n", m_Superblock->MinorVersion);
		printf("LaskFSCKTime = %d\n", m_Superblock->LaskFSCKTime);
		printf("FSCKInterval = %d\n", m_Superblock->FSCKInterval);
		printf("OSID = %d\n", m_Superblock->OSID);
		printf("MajorVersion = %d\n", m_Superblock->MajorVersion);
		printf("UserID = %d\n", m_Superblock->UserID);
		printf("GroupID = %d\n", m_Superblock->GroupID);
		printf("FirstInode = %d\n", m_Superblock->FirstInode);
		printf("InodeSize = %d\n", m_Superblock->InodeSize);
		printf("BlockGroup = %d\n", m_Superblock->BlockGroup);
		printf("OptionalFeatures = %d\n", m_Superblock->OptionalFeatures);
		printf("RequiredFeatues = %d\n", m_Superblock->RequiredFeatues);
		printf("UnsupportedFeatures = %d\n", m_Superblock->UnsupportedFeatures);
		printf("FileSystemID = %s\n", m_Superblock->FileSystemID);
		printf("VolumeName = %s\n", m_Superblock->VolumeName);
		printf("PathVolumeLastMounted = %s\n", m_Superblock->PathVolumeLastMounted);
		printf("CompressionAlgorithm = %d\n", m_Superblock->CompressionAlgorithm);
		printf("PreallocateBlockCountFile = %d\n", m_Superblock->PreallocateBlockCountFile);
		printf("PreallocateBlockCountDir = %d\n", m_Superblock->PreallocateBlockCountDir);
		printf("JournalID = %s\n", m_Superblock->JournalID);
		printf("JournalDevice = %d\n", m_Superblock->JournalDevice);
		printf("OrphanInodeListHead = %d\n", m_Superblock->OrphanInodeListHead);
		printf("JournalInode = %d\n", m_Superblock->JournalInode);
		printf("-----EXT2 DUMP SUPERBLOCK END-----\n");
	}

	void EXT2FileSystem::DumpBlockGroupDescriptors()
	{
		printf("\n-----EXT2 DUMP (%d) BLOCKGROUP DESCRIPTOR START-----\n", m_BlockGroupDescriptorCount);
		for (uint32_t i = 0; i < m_BlockGroupDescriptorCount; i++)
		{
			printf("---START BlockGroup(%d)---\n", i);
			printf("BlockBitmap = %d\n", m_BlockGroupDescriptors[i].BlockBitmap);
			printf("BlockBitmap = %d\n", m_BlockGroupDescriptors[i].BlockBitmap);
			printf("InodeTable = %d\n", m_BlockGroupDescriptors[i].InodeTable);
			printf("FreeBlockCount = %d\n", m_BlockGroupDescriptors[i].FreeBlockCount);
			printf("FreeInodeCount = %d\n", m_BlockGroupDescriptors[i].FreeInodeCount);
			printf("DirCount = %d\n", m_BlockGroupDescriptors[i].DirCount);
			printf("---END BlockGroup(%d)---\n", i);
		}
		printf("-----EXT2 DUMP (%d) BLOCKGROUP DESCRIPTOR END-----\n", m_BlockGroupDescriptorCount);
	}

	void EXT2FileSystem::DumpInode(const Ref<EXT2Inode>& inodebuf)
	{
		printf("\n-----EXT2 DUMP INODE START-----\n");
		printf("Mode = %d\n", inodebuf->Mode);
		printf("UserID = %d\n", inodebuf->UserID);
		printf("SizeLow = %d\n", inodebuf->SizeLow);
		printf("LastAccessTime = %d\n", inodebuf->LastAccessTime);
		printf("LastCreationTime = %d\n", inodebuf->LastCreationTime);
		printf("LastModificationTime = %d\n", inodebuf->LastModificationTime);
		printf("DeletionTime = %d\n", inodebuf->DeletionTime);
		printf("GroupID = %d\n", inodebuf->GroupID);
		printf("HardLinkCount = %d\n", inodebuf->HardLinkCount);
		printf("SectorCount = %d\n", inodebuf->SectorCount);
		printf("Flags = %d\n", inodebuf->Flags);
		printf("OSSV1 = %d\n", inodebuf->OSSV1);
		printf("DPB = %d\n", inodebuf->DBP);
		printf("DPB[0] = %d\n", inodebuf->DBP[0]);
		printf("SIBP = %d\n", inodebuf->SIBP);
		printf("DIBP = %d\n", inodebuf->DIBP);
		printf("TIBP = %d\n", inodebuf->TIBP);
		printf("FragmentAddr = %d\n", inodebuf->FragmentAddr);
		printf("OSSV2 = %d\n", inodebuf->OSSV2);
		printf("-----EXT2 DUMP INODE END-----\n");
	}

	void EXT2FileSystem::DumpInodeNodes(const Ref<EXT2Inode>& inodebuf)
	{
		if (!(inodebuf->Mode & (int)InodeType::Directory)) return;

		for (uint32_t i = 0; i < EXT2_DBP_SIZE; i++)
		{
			if (!inodebuf->DBP[i]) continue;

			uint8_t* data = ReadBlock(inodebuf->DBP[i]);
			EXT2Dirent* dir = (EXT2Dirent*)data;

			if (dir->Size == 0 || dir->NameLength == 0 || dir->Inode <= 0 || dir->Inode >= m_Superblock->InodeCount)
			{
				delete data;
				break;
			}

			while (dir->NameLength > 0 && dir->Inode > 0 && dir->Inode < m_Superblock->InodeCount)
			{
				DumpDirent(dir);
				dir = (EXT2Dirent*)((uint8_t*)dir + dir->Size);
			}

			delete data;
		}
	}

	void EXT2FileSystem::DumpDirent(const EXT2Dirent* dir)
	{
		printf("\n-----EXT2 DUMP Dirent START-----\n");
		printf("Inode = %d\n", dir->Inode);
		printf("Size = %d\n", dir->Size);
		printf("NameLength = %d\n", dir->NameLength);
		printf("Type = %d\n", dir->Type);
		char* tmpname = new char[dir->NameLength + 1];
		memcpy(tmpname, dir->Name, dir->NameLength);
		tmpname[dir->NameLength] = 0;
		printf("Name = %s\n", tmpname);
		delete[] tmpname;
		printf("-----EXT2 DUMP Dirent END-------\n");
	}
}
