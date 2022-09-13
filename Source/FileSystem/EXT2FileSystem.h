#pragma once

#include "FileSystem/EXT2Superblock.h"
#include "FileSystem/EXT2Inode.h"
#include "Util/Memory.h"

#include <string>
#include <memory>

namespace EXT2FS
{
	class EXT2FileSystem
	{
	public:
		bool Init();
		void Destroy();
		bool OpenDevice(const std::string& devpath);
		void LS(const std::string& path);
		void CD(const std::string& path);

	private:
		bool ParseSuperblock();
		bool ParseBlockGroups();
		const EXT2BlockGroupDescriptor& GetBlockGroup(uint32_t group);
		Ref<EXT2Inode> GetInode(uint32_t inode);
		Ref<EXT2Inode> GetInodeFromDir(const Ref<EXT2Inode>& inodebuf, const std::string& name);
		Ref<EXT2Inode> GetInodeFromPath(const std::string& path);

		uint8_t* ReadBlock(uint32_t block);

	public:
		// DEBUG
		void DumpSuperblock();
		void DumpBlockGroupDescriptors();
		void DumpInode(const Ref<EXT2Inode>& inodebuf);
		void DumpInodeNodes(const Ref<EXT2Inode>& inodebuf);
		void DumpDirent(const EXT2Dirent* dir);

	private:
		std::string m_Device;
		std::string m_CurrentPath;

	private:
		std::unique_ptr<EXT2SuperBlock> m_Superblock;
		uint32_t m_FirstBlockGroup;
		uint32_t m_BlockGroupDescriptorCount;
		std::vector<EXT2BlockGroupDescriptor> m_BlockGroupDescriptors;
		Ref<EXT2Inode> m_RootInode;
	};
}
