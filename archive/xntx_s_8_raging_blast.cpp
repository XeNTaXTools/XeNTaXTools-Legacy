/*
=============================================================================
Module Information
------------------
Name:			model_capmodb.cpp
Author:			Rich Whitehouse
Description:	Capcom .mod, used in MvC3
=============================================================================
*/

#include <fcntl.h>
#include <sys/stat.h>
#include <direct.h>
#include <string.h>
#include <io.h>
// #include "main.h"
// #include "glwrap.h"
// #include "cnt_pgeo.h"
#include "shared_types.h"

mathImpFn_t *g_mfn = NULL;
noePluginFn_t *g_nfn = NULL;

/*
The entire contents of a file (360) are:

sizeof(capModBHdr_t) + sizeof(capBoneInfo_t)*hdr->boneNum + sizeof(float)*4*4*hdr->boneNum + sizeof(float)*4*4*hdr->boneNum + 256 +
32*hdr->unknownThingNum + 128*hdr->materialNum + g_modObjSize*hdr->objNum + sizeof(int) + sizeof(capModGroupB_t)*numGroupsB + hdr->vertDataSizeA + hdr->idxNum*2
-
(header, bone infos, relative bone matrices, absolute bone matrices, 256-byte index map, unknown things that are 32 bytes each,
material/meshname entries each a 128 byte string, mesh objects, int representing number of bgroups, bgroups, vert data, index data)
*/

//extern noesisExtTexRef_t *Noesis_AllocTexRefs(char *diffuse, char *normal, char *specular, char *opacity);
//extern noesisAnim_t *rpgCreateProceduralAnim(modelBone_t *bones, int numBones, sharedPAnimParm_t *parms, int numParms, int numFrames);

#pragma pack(push, 1)

enum DataType
{
	DT_AFS = 0x41465300, // 'AFS'
	DT_STPZ = 0x5354505A, // 'STPZ'
	DT_SCD0 = 0x53434430, // 'SCD0'
	DT_SCL0 = 0x53434C30, // 'SCL0'
	DT_ADX = 0x80000024, // 'ADX'
	DT_STPK = 0x5354504B, // 'STPK'
	DT_SPR3 = 0x53505233, // 'SPR3'
	DT_SPRP = 0x53505250, // 'SPRP'
	DT_SPR = 0x53505200, // 'SPRP'
	DT_TX2D = 0x54583244, // 'TX2D'
	DT_MTRL = 0x4D54524C, // 'MTRL'
	DT_SHAP = 0x53484150, // 'SHAP'
	DT_VBUF = 0x56425546, // 'VBUF'
	DT_SCNE = 0x53434E45, // 'SCNE'
	DT_BONE = 0x424F4E45, // 'BONE'
	DT_DRVN = 0x4452564E, // 'DRVN'
	DT_TXAN = 0x5458414E, // 'TXAN'
};

enum DataIndex
{
	DI_TX2D = 0, // 'TX2D'
	DI_MTRL, // 'MTRL'
	DI_SHAP, // 'SHAP'
	DI_VBUF, // 'VBUF'
	DI_SCNE, // 'SCNE'
	DI_BONE, // 'BONE'
	DI_DRVN, // 'DRVN'
	DI_TXAN, // 'TXAN'

	DI_COUNT,
};

struct SPRP_HEADER
{
	// 0x00
	uint32 dataTag; // "SPRP"
	uint16 unknown0x04;
	uint16 unknown0x06;
	uint32 entryCount;
	uint32 unknown0x0C;
	// 0x10
	uint32 nameOffset;
	uint32 entryInfoSize;
	uint32 stringTableSize;
	uint32 dataInfoSize;
	// 0x20
	uint32 dataBlockSize;
	uint32 ioramNameOffset;
	uint32 ioramDataSize;
	uint32 vramNameOffset;
	// 0x30
	uint32 vramDataSize;
	uint32 unknown0x34[3];
	// 0x40
};

struct SPRP_TYPE_ENTRY
{
	// 0x00
	uint32 dataType;
	uint32 unknown0x04;
	uint32 dataCount;
	// 0x0C
};

struct SPRP_DATA_INFO
{
	// 0x00
	uint32 nameOffset;
	uint32 dataOffset;
	uint32 dataSize;
	uint32 childCount;
	// 0x10
	uint32 childOffset;
	// 0x14
};

struct SPRP_DATA_ENTRY
{
	// 0x00
	uint32 dataType;
	uint32 index;

	SPRP_DATA_INFO dataInfo;

	uint32 unknown0x1C;
	// 0x20
};

struct MTRL_LAYER
{
	uint32 layerName;
	uint32 sourceName;
};

struct MTRL_INFO
{
	// 0x00
	uint32 unknown0x00[0x70 >> 2];
	// 0x70
	MTRL_LAYER layers[10];
	// 0xC0
};

struct TX2D_INFO
{
	// 0x00
	uint32 unknown0x00;
	uint32 dataOffset;
	uint32 unknown0x08;
	uint32 dataSize;
	// 0x10
	uint16 width;
	uint16 height;
	uint16 unknown0x14;
	uint16 mipmapCount;
	uint32 unknown0x18;
	uint32 unknown0x1C;
	// 0x20
	uint8  unknown0x20[4];
	// 0x24
};

struct BONE_DATA
{
	// 0x00
	uint32 index;
	uint32 name;
	uint32 childCount;
	uint32 childOffsets;
	// 0x10
	uint32 relativeOffset;
	uint32 absoluteOffset;
	uint32 inverseOffset;

	uint32 unknown0x1C[0x90 >> 2];
	// 0xAC
};

struct BONE_INFO
{
	// 0x00
	uint32 unknown0x00;
	uint32 jointCount;
	uint32 unknown0x08;
	uint32 dataOffset;
	// 0x10
	uint32 relativeOffset;
	uint32 absoluteOffset;
	uint32 inverseOffset;
	uint32 unknown0x1C;
	// 0x20
	uint32 unknown0x20;
	// 0x24
};

enum VertexUsage
{
	VTXUSAGE_POSITION = 0,
	VTXUSAGE_NORMAL   = 2,
	VTXUSAGE_TEXCOORD = 5,
	VTXUSAGE_WEIGHTS  = 6,
	VTXUSAGE_INDICES  = 7,
};

enum VertexFormat
{
	VTXFMT_ULONG1 =  2,
	VTXFMT_FLOAT1 =  8,
	VTXFMT_FLOAT2 =  9,
	VTXFMT_FLOAT3 = 10,
	VTXFMT_FLOAT4 = 11,

	VTXFMT_COUNT,
};

int fmtDataType[VTXFMT_COUNT] = {0, 0, RPGEODATA_UINT, 0, 0, 0, 0, 0, RPGEODATA_FLOAT, RPGEODATA_FLOAT, RPGEODATA_FLOAT, RPGEODATA_FLOAT};
uint32 fmtDataSize[VTXFMT_COUNT] = {0, 0, 4, 0, 0, 0, 0, 0, 4, 4, 4, 4};

struct VERTEX_DECL
{
	// 0x00
	uint32 unknown0x00;
	uint32 resourceName;
	uint16 usage;
	uint16 index;
	uint16 format;
	uint16 stride;
	// 0x10
	uint32 offset;
	// 0x14
};

struct VBUF_INFO
{
	// 0x00
	uint32 unknown0x00;
	uint32 unknown0x04;
	uint32 dataOffset;
	uint32 dataSize;
	// 0x10
	uint32 vertexCount;
	uint16 unknown0x14;
	uint16 unknown0x16;
	uint16 declCount0;
	uint16 declCount1;
	uint32 declOffset;
	// 0x20
};

struct SCNE_MODEL
{
	uint32 unknown0x00;
	uint32 typeName;
	uint32 resourceName;
	uint32 layerName;
	uint32 parentName;
};

struct SCNE_MATERIAL_INFO
{
	uint32 layerName;
	uint32 targetName;
	uint32 unknown0x08;
};

struct SCNE_MATERIAL
{
	// 0x00
	uint32 materialName;
	uint32 unknown0x04;
	uint32 unknown0x08;

	SCNE_MATERIAL_INFO materialInfo[1];
};

#pragma pack(pop)

#pragma pack (push, 1)

struct AFS_HEADER
{
	uint32 fileTag; // 'AFS'
	uint32 fileCount;	
};

struct AFS_FILEINFO
{
	uint32 fileOffset;
	uint32 fileSize;
};

struct STPZ_HEADER
{
	uint32 fileTag; // "STPZ"
	uint32 unknown0x04;
	uint32 unknown0x08;
	uint32 unknown0x0C;
};

struct _0DCS_HEADER
{
	uint32 fileTag; // "0DCS"
	uint32 uncompressedSize;
	uint32 compressedSize;
	uint32 unknown0x0C; // maximum uncompressed block size? - have not seen '0LCS' sections bigger than this size, but not sure if this is related
};

struct _0LCS_HEADER
{
	// 0x00
	uint32 fileTag; // "0LCS"
	uint32 uncompressedSize;
	uint32 compressedSize;
	uint32 unknown0x0C;
	// 0x10
};

struct STPK_ENTRY
{
	// 0x00
	uint32 dataOffset;
	uint32 dataSize;
	uint32 unknown0x08;
	uint32 unknown0x0C;
	// 0x10
	char   name[32];
	// 0x30
};

struct STPK_HEADER
{
	// 0x00
	uint32 dataTag; // "STPK"
	uint32 unknown0x04;
	uint32 entryCount;
	uint32 unknown0x0C;
	// 0x10
	STPK_ENTRY dataEntries[1];
};


static uint8 srcBuffer[0x4000] = {0};
static uint8 dstBuffer[0x4000] = {0};

#pragma pack (pop)

//is it a capcom .mod?
bool Model_SPR_Check(BYTE *fileBuffer, int bufferLen, noeRAPI_t *rapi)
{
	if (bufferLen < sizeof(SPRP_HEADER))
	{
		return false;
	}

	SPRP_HEADER *hdr = (SPRP_HEADER *)fileBuffer;
	if ((GetBigInt(hdr->dataTag) == DT_SPR3 ||
		(GetBigInt(hdr->dataTag) & (~0xFF)) == DT_SPR) &&
		GetBigInt(hdr->entryCount) > 0 &&
		(sizeof(SPRP_HEADER) +
		GetBigInt(hdr->entryInfoSize) +
		GetBigInt(hdr->stringTableSize) +
		GetBigInt(hdr->dataInfoSize) +
		GetBigInt(hdr->dataBlockSize)) <= bufferLen)
	{
		return true;
	}

	return false;
}

void unpackSTPKData(std::vector<uint8>& outData, noeRAPI_t *rapi)
{
	std::string path = "";
	uint8* data = &outData.front();
	uint32 size = outData.size();

	std::list<std::string> dataPaths;
	std::list<uint8*> dataOffsets;
	std::list<uint32> dataSizes;

	dataPaths.push_back(path);
	dataOffsets.push_back(data);
	dataSizes.push_back(size);

	while (!dataOffsets.empty())
	{
		data = dataOffsets.front(); dataOffsets.pop_front();
		path = dataPaths.front(); dataPaths.pop_front();
		size = dataSizes.front(); dataSizes.pop_front();

		STPK_HEADER *dataHeader = (STPK_HEADER*)data;

		LITTLE_BIG_SWAP(dataHeader->dataTag);
		LITTLE_BIG_SWAP(dataHeader->entryCount);

		if (dataHeader->dataTag == DT_STPK)
		{
			uint32 entryNum = 0;
			for (entryNum = 0; entryNum < dataHeader->entryCount; ++entryNum)
			{
				STPK_ENTRY& entry = dataHeader->dataEntries[entryNum];

				LITTLE_BIG_SWAP(entry.dataOffset);
				LITTLE_BIG_SWAP(entry.dataSize);

				char n[33] = {0};
				memcpy(n, entry.name, 32);

				if ((entry.dataSize > 0) && (strlen(n) < 32))
				{
					char *ext = strrchr(n, '.');

					if (ext && stricmp(".pak", ext) == 0)
					{
						*ext = 0;

						dataOffsets.push_back(data + entry.dataOffset);
						dataPaths.push_back(path + n + '\\');
						dataSizes.push_back(entry.dataSize);
					}
					else
					{
						std::string fpath = (path + n);
						//write it out
						rapi->LogOutput("Writing '%s'.\n", fpath.c_str());
						rapi->Noesis_ExportArchiveFile(const_cast<char*>(fpath.c_str()), data + entry.dataOffset, entry.dataSize);
					}
				}
			}
		}
		else
		{
			std::string fpath = path;
			fpath.replace(fpath.length() - 1, 1, ".pak");

			LITTLE_BIG_SWAP(dataHeader->dataTag);
			LITTLE_BIG_SWAP(dataHeader->entryCount);

			//write it out
			rapi->LogOutput("Writing '%s'.\n", fpath.c_str());
			rapi->Noesis_ExportArchiveFile(const_cast<char*>(fpath.c_str()), data, size);
		}
	}
}

void unpackSTPZData(const int& infile, std::vector<uint8>& outData)
{
	STPZ_HEADER fileHeader = {0};
	_read(infile, &fileHeader, sizeof(fileHeader));

	LITTLE_BIG_SWAP(fileHeader.fileTag);

	if (fileHeader.fileTag == DT_STPZ)
	{
		_0DCS_HEADER scdHeader = {0};
		_read(infile, &scdHeader, sizeof(scdHeader));

		if (scdHeader.fileTag == DT_SCD0)
		{
			uint32 offset = 0;

			outData.resize(scdHeader.uncompressedSize);

			uint32 totalSize = sizeof(scdHeader);
			while (totalSize < scdHeader.compressedSize)
			{
				_0LCS_HEADER sclHeader = {0};
				_read(infile, &sclHeader, sizeof(sclHeader));

				if (sclHeader.fileTag == DT_SCL0)
				{
					_read(infile, srcBuffer, sclHeader.compressedSize - sizeof(sclHeader));

					uint8* src = srcBuffer;
					uint8* end = src + (sclHeader.compressedSize - sizeof(sclHeader));
					uint8* dst = dstBuffer;
					uint8* src2 = src;

					while (src < end)
					{
						uint8 v0 = *src;
						++src;

						uint8 v1 = (v0 >> 1) & 0x7F;
						uint8 v2 = (v1 >> 2);
						uint8 v3 = (v1 & 3);

						if (v0 & 1)
						{
							uint8 v4 = *src;
							++src;

							src2 = dst - v4;
						}
						else
						{
							src2 = src;
						}

						int i = 0;
						for (i = 0; i < v2; ++i)
						{
							dst[0] = src2[0];
							dst[1] = src2[1];
							dst[2] = src2[2];
							dst[3] = src2[3];

							src2 += 4;
							dst += 4;
						}

						for (i = 0; i < v3; ++i)
						{
							dst[0] = src2[0];

							++src2;
							++dst;
						}

						if (!(v0 & 1))
						{
							src = src2;
						}
					}

					//_write(outfile, dstBuffer, sclHeader.uncompressedSize);
					memcpy(&outData[offset], dstBuffer, sclHeader.uncompressedSize);
					offset += sclHeader.uncompressedSize;
				}

				totalSize += sclHeader.compressedSize;
			}
		}
	}
}

//is it a capcom .mod?
bool Model_AFS_ExportFiles(int infile, int64 fileLen, bool justChecking, noeRAPI_t *rapi)
{
	_lseeki64(infile, 0, SEEK_SET);

	if (fileLen < sizeof(AFS_HEADER))
	{
		return false;
	}

	AFS_HEADER fileHeader;
	_read(infile, &fileHeader, sizeof(fileHeader));

	LITTLE_BIG_SWAP(fileHeader.fileTag);

	if (DT_AFS != fileHeader.fileTag)
	{
		return false;
	}

	if (fileLen < (sizeof(AFS_HEADER) + (sizeof(AFS_FILEINFO) * fileHeader.fileCount)))
	{
		return false;
	}

	std::vector<AFS_FILEINFO> fileInfo;
	fileInfo.resize(fileHeader.fileCount);

	_read(infile, &fileInfo.front(), sizeof(AFS_FILEINFO) * fileHeader.fileCount);

	AFS_FILEINFO *finfo = &fileInfo[fileHeader.fileCount - 1];
	if (fileLen < (finfo->fileOffset + finfo->fileSize))
	{
		return false;
	}

	if (justChecking)
	{
		return true;
	}

	char fileName[MAX_PATH] = "";

	uint32 fileNum = 0;
	for (fileNum = 0; fileNum < fileHeader.fileCount; ++fileNum)
	{
		uint32 fileTag = 0;

		finfo = &fileInfo[fileNum];

		_lseeki64(infile, finfo->fileOffset, SEEK_SET);
		_read(infile, &fileTag, 4);
		_lseeki64(infile, finfo->fileOffset, SEEK_SET);

		LITTLE_BIG_SWAP(fileTag);

		switch (fileTag)
		{
		case DT_STPZ:
			{
				std::vector<uint8> stpkData;
				sprintf(fileName, "%04d.stpk", fileNum);
				unpackSTPZData(infile, stpkData);
				unpackSTPKData(stpkData, rapi);
				continue;
			}
			break;

		case DT_ADX:
			{
				sprintf(fileName, "%04d.adx", fileNum);
			}
			break;

		default:
			{
				sprintf(fileName, "%04d.raw", fileNum);
			}
			break;
		}

		BYTE *rawData = (BYTE *)rapi->Noesis_UnpooledAlloc(finfo->fileSize);

		_read(infile, rawData, finfo->fileSize);

		//write it out
		rapi->LogOutput("Writing '%s'.\n", fileName);
		rapi->Noesis_ExportArchiveFile(fileName, rawData, finfo->fileSize);
		rapi->Noesis_UnpooledFree(rawData);
	}

	return true;
}

typedef std::map<std::string, int> StringMap;

//when using the stream archive handler, you are responsible for managing the file handle yourself.
bool Model_AFS_Export(wchar_t *filename, __int64 len, bool justChecking, noeRAPI_t *rapi)
{
	if (len < sizeof(AFS_HEADER))
	{
		return false;
	}
	int infile = _wopen(filename, _O_RDONLY | _O_BINARY);
	if (infile < 0)
	{
		return false;
	}

	bool r = Model_AFS_ExportFiles(infile, len, justChecking, rapi);

	_close(infile);

	return r;
}

//load capcom .mod
//extern bool g_capTexBDef360;
noesisModel_t *Model_SPR_Load(BYTE *fileBuffer, int bufferLen, int &numMdl, noeRAPI_t *rapi)
{
	SPRP_HEADER* sprHeader = (SPRP_HEADER*)(fileBuffer);

	uint8* typeInfoBase = (uint8*)(fileBuffer + sizeof(SPRP_HEADER));
	uint8* stringBase   = (uint8*)(typeInfoBase + GetBigInt(sprHeader->entryInfoSize));
	uint8* dataInfoBase = (uint8*)(stringBase + GetBigInt(sprHeader->stringTableSize));
	uint8* dataBase     = (uint8*)(dataInfoBase + GetBigInt(sprHeader->dataInfoSize));

	const char* ioramName = (const char*)(stringBase + GetBigInt(sprHeader->ioramNameOffset));
	const char* vramName = (const char*)(stringBase + GetBigInt(sprHeader->vramNameOffset));

	int ioramSize = 0;
	uint8* ioramBuffer = (uint8*)rapi->Noesis_LoadPairedFile("ioram File", "*.ioram", ioramSize, NULL);

	int vramSize = 0;
	uint8* vramBuffer = (uint8*)rapi->Noesis_LoadPairedFile("vram File", "*.vram", vramSize, NULL);

// 	if (!ioramBuffer || !vramBuffer)
// 	{
// 		if (ioramBuffer)
// 			rapi->Noesis_UnpooledFree(ioramBuffer);
// 
// 		if (vramBuffer)
// 			rapi->Noesis_UnpooledFree(vramBuffer);
// 
// 		return NULL;
// 	}

	SPRP_TYPE_ENTRY* entryInfo = (SPRP_TYPE_ENTRY*)(typeInfoBase);
	uint32 infoOffset = 0;
	uint32 infoNum = 0;
	const int entryCount = GetBigInt(sprHeader->entryCount);

	SPRP_TYPE_ENTRY* typeInfo[DI_COUNT] = {NULL};
	SPRP_DATA_ENTRY* dataInfo[DI_COUNT] = {NULL};

	for (infoNum = 0; infoNum < entryCount; ++infoNum)
	{
		SPRP_DATA_ENTRY* dataEntry = (SPRP_DATA_ENTRY*)(dataInfoBase + infoOffset);
		DataIndex index = DI_COUNT;

		switch (GetBigInt(entryInfo[infoNum].dataType))
		{
		case DT_TX2D: { index = DI_TX2D; } break;
		case DT_MTRL: { index = DI_MTRL; } break;
		case DT_VBUF: { index = DI_VBUF; } break;
		case DT_SCNE: { index = DI_SCNE; } break;
		case DT_BONE: { index = DI_BONE; } break;
		}

		if (index < DI_COUNT)
		{
			typeInfo[index] = (entryInfo + infoNum);
			dataInfo[index] = dataEntry;
		}

		infoOffset += GetBigInt(entryInfo[infoNum].dataCount) * sizeof(SPRP_DATA_ENTRY);
	}

	StringMap textureIndices;

	CArrayList<noesisTex_t *> textures;
	CArrayList<noesisMaterial_t *> materials;

	if (typeInfo[DI_TX2D] && vramBuffer)
	{
		uint32 dataNum = 0;
		const int dataCount = GetBigInt(typeInfo[DI_TX2D]->dataCount);
		SPRP_DATA_ENTRY* dataEntry = dataInfo[DI_TX2D];

		for (dataNum = 0; dataNum < dataCount; ++dataNum, ++dataEntry)
		{
			char* name = (char*)(stringBase + GetBigInt(dataEntry->dataInfo.nameOffset));
			TX2D_INFO* info = (TX2D_INFO*)(dataBase + GetBigInt(dataEntry->dataInfo.dataOffset));

			int fmt = NOESISTEX_RGBA32;
// 			switch (GetBigInt(info->unknown0x00))
// 			{
// 			case 0x02:
// 				fmt = NOESISTEX_DXT1;
// 				break;
// 			case 0x22:
// 				fmt = NOESISTEX_DXT5;
// 				break;
// 			default:
// 				continue;
// 				break;
// 			}
			switch (info->unknown0x20[0])
			{
			case 0x00:
				fmt = NOESISTEX_RGBA32;
				break;
			case 0x08:
				fmt = NOESISTEX_DXT1;
				break;
			case 0x18:
			case 0x20:
				fmt = NOESISTEX_DXT5;
				break;
			default:
				continue;
				break;
			}

			uint8 *srcImage = (uint8 *)(vramBuffer + GetBigInt(info->dataOffset));
			uint8 *tempImage = (uint8 *)rapi->Noesis_UnpooledAlloc(GetBigInt(info->dataSize));

			if (GetBigInt(sprHeader->dataTag) == DT_SPR3)
			{
				int mipSize = (fmt == NOESISTEX_DXT1) ? (GetBigWord(info->width) * GetBigWord(info->height))/2 : (GetBigWord(info->width) * GetBigWord(info->height));
				rapi->Noesis_UntileImageDXT((BYTE*)tempImage, (BYTE*)srcImage, GetBigInt(info->dataSize), GetBigWord(info->width), GetBigWord(info->height), (fmt == NOESISTEX_DXT1) ? 8 : 16);

				uint32 blockOffset = 0;
				for (; blockOffset < GetBigInt(info->dataSize); blockOffset += 2)
				{
					LittleBigSwap(tempImage + blockOffset, 2);
				}
			}
			else
			{
				memcpy(tempImage, srcImage, GetBigInt(info->dataSize));
			}

			// create texture
			noesisTex_t* nt = rapi->Noesis_TextureAlloc(name, GetBigWord(info->width), GetBigWord(info->height), tempImage, fmt);
			nt->shouldFreeData = true;
			textures.Append(nt);

			textureIndices.insert(StringMap::value_type(name, GetBigInt(dataEntry->index)));
		}
	}

	if (typeInfo[DI_MTRL])
	{
		uint32 dataNum = 0;
		const int dataCount = GetBigInt(typeInfo[DI_MTRL]->dataCount);
		SPRP_DATA_ENTRY* dataEntry = dataInfo[DI_MTRL];

		for (dataNum = 0; dataNum < dataCount; ++dataNum, ++dataEntry)
		{
			char* name = (char*)(stringBase + GetBigInt(dataEntry->dataInfo.nameOffset));
			MTRL_INFO* info = (MTRL_INFO*)(dataBase + GetBigInt(dataEntry->dataInfo.dataOffset));

			noesisMaterial_t* nmat = rapi->Noesis_GetMaterialList(1, true);
			nmat->name = rapi->Noesis_PooledString(name);
			nmat->noDefaultBlend = true;
			//nmat->noLighting = true;
			nmat->texIdx = dataNum;

			int layerNum = 0;
			for (layerNum = 0; layerNum < 10; ++layerNum)
			{
				char* layerName = (char*)(stringBase + GetBigInt(info->layers[layerNum].layerName));
				if (0 == strnicmp("COLORMAP", layerName, 8))
				{
					char* sourceName = (char*)(stringBase + GetBigInt(info->layers[layerNum].sourceName));

					StringMap::iterator itMat = textureIndices.find(sourceName);
					if (textureIndices.end() != itMat)
					{
						nmat->extRefs = rapi->Noesis_AllocTexRefs(sourceName, NULL, NULL, NULL);
						nmat->texIdx = itMat->second;
						break;
					}
				}
			}
			materials.Append(nmat);
		}
	}

	StringMap vbufferIndices;
	if (typeInfo[DI_VBUF])
	{
		uint32 dataNum = 0;
		const int dataCount = GetBigInt(typeInfo[DI_VBUF]->dataCount);
		SPRP_DATA_ENTRY* dataEntry = dataInfo[DI_VBUF];

		for (dataNum = 0; dataNum < dataCount; ++dataNum, ++dataEntry)
		{
			char* name = (char*)(stringBase + GetBigInt(dataEntry->dataInfo.nameOffset));
			VBUF_INFO* info = (VBUF_INFO*)(dataBase + GetBigInt(dataEntry->dataInfo.dataOffset));

			vbufferIndices.insert(StringMap::value_type(name, dataNum));
		}
	}

	void *pgctx = rapi->rpgCreateContext();
	rapi->rpgSetEndian(true);

	float zero[] = {0,0,0,1};
	rapi->rpgBegin(RPGEO_TRIANGLE);
	rapi->rpgVertex3f(zero);
	rapi->rpgVertex3f(zero);
	rapi->rpgVertex3f(zero);
	rapi->rpgEnd();

	int numBones = 0;
	modelBone_t* bones = NULL;
	if (typeInfo[DI_BONE])
	{
		uint32 dataNum = 0;
		const int dataCount = GetBigInt(typeInfo[DI_BONE]->dataCount);
		SPRP_DATA_ENTRY* dataEntry = dataInfo[DI_BONE];

		uint8* data = (uint8*)(dataBase + GetBigInt(dataEntry->dataInfo.dataOffset));
		BONE_INFO* info = (BONE_INFO*)(data);

		BONE_DATA* boneData = (BONE_DATA*)(data + GetBigInt(info->dataOffset));

		numBones = GetBigInt(info->jointCount);

		if (numBones > 0)
		{
			bones = rapi->Noesis_AllocBones(numBones);

			rapi->rpgBegin(RPGEO_TRIANGLE);
			int boneNum = 0;
			for (boneNum = 0; boneNum < numBones; ++boneNum)
			{
				const char* name = (const char*)(data + GetBigInt(boneData[boneNum].name));
				strcpy_s(bones[boneNum].name, 31, name);

				RichMat44 m((float*)(data + GetBigInt(boneData[boneNum].absoluteOffset)));
				m.ChangeEndian();

				g_mfn->Math_4x4ToModelMat(&m.m, &bones[boneNum].mat);

				rapi->rpgVertex3f(m.m.c4);
				rapi->rpgVertex3f(m.m.c4);
				rapi->rpgVertex3f(m.m.c4);

				const int childCount = GetBigInt(boneData[boneNum].childCount);
				if (childCount > 0)
				{
					uint32* childOffsets = (uint32*)(data + GetBigInt(boneData[boneNum].childOffsets));
					int childNum = 0;
					for (childNum = 0; childNum < childCount; ++childNum)
					{
						BONE_DATA* child = (BONE_DATA*)(data + GetBigInt(childOffsets[childNum]));
						modelBone_t* cb = (bones + GetBigInt(child->index));
						cb->eData.parent = &bones[boneNum];
					}
				}
			}
			rapi->rpgEnd();
		}
	}

	noesisMatData_t *md = rapi->Noesis_GetMatDataFromLists(materials, textures);
	rapi->rpgSetExData_Materials(md);
	if (numBones > 0)
	{
		rapi->rpgSetExData_Bones(bones, numBones);
	}

	CArrayList<noesisModel_t *> models;
// 	for (int i = 0; i < uniqueLods.Num(); i++)
// 	{
// 		noesisModel_t *mdl = Model_CapModB_ParseModelLod(matList, fileBuffer, bufferLen, hdr, objects, materials, bones, boneRefMaps, uniqueLods[i], rapi);
// 		if (mdl)
// 		{
// 			models.Append(mdl);
// 		}
// 	}

	if (typeInfo[DI_SCNE] && ioramBuffer)
	{
		uint32 dataNum = 0;
		const int dataCount = GetBigInt(typeInfo[DI_SCNE]->dataCount);
		SPRP_DATA_ENTRY* dataEntry = dataInfo[DI_SCNE];

		const int childCount = GetBigInt(dataEntry->dataInfo.childCount);
		int childNum = 0;

		SPRP_DATA_INFO* childEntry = (SPRP_DATA_INFO*)(dataBase + GetBigInt(dataEntry->dataInfo.childOffset));
		SPRP_DATA_INFO* nodeEntry = NULL;
		int nodeCount = 0;

		for (childNum = 0; childNum < childCount; ++childNum, ++childEntry)
		{
			char* name = (char*)(stringBase + GetBigInt(childEntry->nameOffset));
			if (0 == stricmp("[NODES]", name))
			{
				nodeCount = GetBigInt(childEntry->childCount);
				nodeEntry = (SPRP_DATA_INFO*)(dataBase + GetBigInt(childEntry->childOffset));
			}
		}

		if (nodeCount > 0)
		{
			int nodeNum = 0;
			for (nodeNum = 0; nodeNum < nodeCount; ++nodeNum, ++nodeEntry)
			{
				char* name = (char*)(stringBase + GetBigInt(nodeEntry->nameOffset));
				if (0 == strnicmp("|model", name, 6))
				{
					int childCount = GetBigInt(nodeEntry->childCount);
					int childNum = 0;
					SPRP_DATA_INFO* nodeChild = (SPRP_DATA_INFO*)(dataBase + GetBigInt(nodeEntry->childOffset));

					for (childNum = 0; childNum < childCount; ++childNum, ++nodeChild)
					{
						name = (char*)(stringBase + GetBigInt(nodeChild->nameOffset));
						if (0 == stricmp("[MATERIAL]", name))
						{
							SCNE_MATERIAL* m = (SCNE_MATERIAL*)(dataBase + GetBigInt(nodeChild->dataOffset));
							name = (char*)(stringBase + GetBigInt(m->materialName));

							rapi->rpgSetMaterial(name);

							break;
						}
					}

					SCNE_MODEL* modelInfo = (SCNE_MODEL*)(dataBase + GetBigInt(nodeEntry->dataOffset));
					name = (char*)(stringBase + GetBigInt(modelInfo->typeName));
					if (0 == strnicmp("mesh", name, 4))
					{
						name = (char*)(stringBase + GetBigInt(modelInfo->resourceName));

						StringMap::iterator itMesh = vbufferIndices.find(name);
						if (vbufferIndices.end() != itMesh)
						{
							int vbufferIndex = itMesh->second;
							SPRP_DATA_ENTRY* vbufferEntry = dataInfo[DI_VBUF] + vbufferIndex;

							VBUF_INFO* vbufferInfo = (VBUF_INFO*)(dataBase + GetBigInt(vbufferEntry->dataInfo.dataOffset));
							uint8* vertexBase = (uint8*)(ioramBuffer + GetBigInt(vbufferInfo->dataOffset));

							VERTEX_DECL* decl = (VERTEX_DECL*)(dataBase + GetBigInt(vbufferInfo->declOffset));
							int declCount = GetBigWord(vbufferInfo->declCount1);
							int declNum = 0;
							int vertexCount = GetBigInt(vbufferInfo->vertexCount);

							for (declNum = 0; declNum < declCount; ++declNum, ++decl)
							{
								uint16 index = GetBigWord(decl->index);
								uint16 format = GetBigWord(decl->format);
								uint16 stride = GetBigWord(decl->stride);
								uint32 offset = GetBigInt(decl->offset);

								switch (GetBigWord(decl->usage))
								{
								case VTXUSAGE_POSITION:
									rapi->rpgBindPositionBuffer(vertexBase + offset, (rpgeoDataType_e)fmtDataType[format], stride);
									break;
								case VTXUSAGE_NORMAL:
									rapi->rpgBindNormalBuffer(vertexBase + offset, (rpgeoDataType_e)fmtDataType[format], stride);
									break;
								case VTXUSAGE_TEXCOORD:
									switch (index)
									{
									case 0:
										rapi->rpgBindUV1Buffer(vertexBase + offset, (rpgeoDataType_e)fmtDataType[format], stride);
										break;
									case 1:
										rapi->rpgBindUV2Buffer(vertexBase + offset, (rpgeoDataType_e)fmtDataType[format], stride);
										break;
									}
									break;
								case VTXUSAGE_WEIGHTS:
									rapi->rpgBindBoneWeightBuffer(vertexBase + offset - (index * fmtDataSize[format]), (rpgeoDataType_e)fmtDataType[format], stride, index + 1);
									break;
								case VTXUSAGE_INDICES:
									rapi->rpgBindBoneIndexBuffer(vertexBase + offset - (index * fmtDataSize[format]), (rpgeoDataType_e)fmtDataType[format], stride, index + 1);
									break;
								}
							}

							uint16* vertexIndices = (uint16*)rapi->Noesis_UnpooledAlloc(vertexCount * sizeof(uint16));
							for (declNum = 0; declNum < vertexCount; ++declNum)
							{
								vertexIndices[declNum] = GetBigWord(declNum);
							}

							rapi->rpgCommitTriangles(vertexIndices, RPGEODATA_USHORT, vertexCount, RPGEO_TRIANGLE_STRIP, false);
							rapi->Noesis_UnpooledFree(vertexIndices);
						}
					}
				}
			}
		}
	}

	rapi->rpgOptimize();

	noesisModel_t *mdl = rapi->rpgConstructModel();
	if (mdl)
	{
		models.Append(mdl);
	}

	rapi->rpgDestroyContext(pgctx);

	if (ioramBuffer)
		rapi->Noesis_UnpooledFree(ioramBuffer);

	if (vramBuffer)
		rapi->Noesis_UnpooledFree(vramBuffer);

	if (models.Num() <= 0)
	{
		return NULL;
	}

	numMdl = models.Num();

	noesisModel_t *mdlList = rapi->Noesis_ModelsFromList(models, numMdl);
	models.Clear();

	return mdlList;
}

//called by Noesis to init the plugin
NPLUGIN_API bool NPAPI_Init(mathImpFn_t *mathfn, noePluginFn_t *noepfn)
{
	g_mfn = mathfn;
	g_nfn = noepfn;

	int th = g_nfn->NPAPI_Register("Dragon Ball Raging Blast model", ".spr");
	if (th < 0)
	{
		return false;
	}

	//set the data handlers for this format
	g_nfn->NPAPI_SetTypeHandler_TypeCheck(th, Model_SPR_Check);
	g_nfn->NPAPI_SetTypeHandler_LoadModel(th, Model_SPR_Load);

	int fh = g_nfn->NPAPI_Register("Dragon Ball Raging Blast Archive", ".afsd");
	if (fh < 0)
	{
		return false;
	}

	//set the data handlers for this format
	g_nfn->NPAPI_SetTypeHandler_ExtractArcStream(fh, Model_AFS_Export);

	return true;
}

//called by Noesis before the plugin is freed
NPLUGIN_API void NPAPI_Shutdown(void)
{
	//nothing to do in this plugin
}

NPLUGIN_API int NPAPI_GetPluginVer(void)
{
	return NOESIS_PLUGIN_VERSION;
}

NPLUGIN_API bool NPAPI_GetPluginInfo(noePluginInfo_t *infOut)
{
	strcpy_s(infOut->pluginName, 64, "raging_blast");
	strcpy_s(infOut->pluginDesc, 512, "Dragon Ball Raging Blast format handler, by revel8n.");
	return true;
}

BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	return TRUE;
}
