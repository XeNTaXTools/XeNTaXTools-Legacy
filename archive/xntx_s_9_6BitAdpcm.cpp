// 6BitAdpcm.cpp : Decompresses UbiSoft's 6-bit ADPCM
//

#include "Pch.h"
#include "6BitAdpcm.h"

long Lookup1[]={
-100000000, -369, -245, -133, -33, 56, 135, 207, 
275, 338, 395, 448, 499, 548, 593, 635, 
676, 717, 755, 791, 825, 858, 889, 919, 
948, 975, 1003, 1029, 1054, 1078, 1103, 1132, 
1800, 1800, 1800, 2048, 3072, 4096, 5000, 5056, 
5184, 5240, 6144, 6880, 9624, 12880, 14952, 18040, 
20480, 22920, 25600, 28040, 32560, 35840, 40960, 45832, 
51200, 56320, 63488, 67704, 75776, 89088, 102400};

long Lookup2[]={
1024, 1031, 1053, 1076, 1099, 1123, 1148, 1172, 
1198, 1224, 1251, 1278, 1306, 1334, 1363, 1393, 
1423, 1454, 1485, 1518, 1551, 1584, 1619, 1654, 
1690, 1726, 1764, 1802, 1841, 1881, 1922, 1964, 
2007, -1024, -1031, -1053, -1076, -1099, -1123, -1148, 
-1172, -1198, -1224, -1251, -1278, -1306, -1334, -1363, 
-1393, -1423, -1454, -1485, -1518, -1551, -1584, -1619, 
-1654, -1690, -1726, -1764, -1802, -1841, -1881, -1922, 
-1964, -2007};

long Lookup3[]={
1800, 1800, 1800, 2048, 3072, 4096, 5000, 5056, 
5184, 5240, 6144, 6880, 9624, 12880, 14952, 18040, 
20480, 22920, 25600, 28040, 32560, 35840, 40960, 45832, 
51200, 56320, 63488, 67704, 75776, 89088, 102400, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 
0, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 2, 2, 3, 3, 4, 
4, 5, 5, 5, 6, 6, 6, 7};

long Lookup1774338[]={
-100000000, 8, 269, 425, 545, 645, 745, 850
};

long Lookup1774378[]={
-1536, 2314, 5243, 8192, 14336, 25354, 45445, 143626, 
0, 0, 0, 1, 1, 1, 3, 7, 
-100000000, -369, -245, -133, -33, 56, 135, 207, 
275, 338, 395, 448, 499, 548, 593, 635, 
676, 717, 755, 791, 825, 858, 889, 919, 
948, 975, 1003, 1029, 1054, 1078, 1103, 1132, 
1800, 1800, 1800, 2048, 3072, 4096, 5000, 5056, 
5184, 5240, 6144, 6880, 9624, 12880, 14952, 18040, 
20480, 22920, 25600, 28040, 32560, 35840, 40960, 45832, 
51200, 56320, 63488, 67704, 75776, 89088, 102400, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 
0, 1, 1, 1, 1, 1, 1, 1, 
1, 1, 1, 2, 2, 3, 3, 4, 
4, 5, 5, 5, 6, 6, 6, 7, 
1024, 1031, 1053, 1076, 1099, 1123, 1148, 1172, 
1198, 1224, 1251, 1278, 1306, 1334, 1363, 1393, 
1423, 1454, 1485, 1518, 1551, 1584, 1619, 1654, 
1690, 1726, 1764, 1802, 1841, 1881, 1922, 1964, 
2007, 0
};

long Lookup1774648[]={
1024, 1031, 1053, 1076, 1099, 1123, 
1148, 1172, 1198, 1224, 1251, 1278, 1306, 1334, 
1363, 1393, 1423, 1454, 1485, 1518, 1551, 1584, 
1619, 1654, 1690, 1726, 1764, 1802, 1841, 1881, 
1922, 1964, 2007, -1024, -1031, -1053, -1076, -1099, 
-1123, -1148, -1172, -1198, -1224, -1251, -1278, -1306, 
-1334, -1363, -1393, -1423, -1454, -1485, -1518, -1551, 
-1584, -1619, -1654, -1690, -1726, -1764, -1802, -1841, 
-1881, -1922, -1964, -2007
};

unsigned long Get6BitAdpcmSamples(unsigned long Bytes)
{
	return Bytes*8/6;
}

unsigned long Get4BitAdpcmSamples(unsigned long Bytes)
{
	return Bytes*2;
}

void Expand6BitAdpcmBlock(void* Source, unsigned long* Dest, unsigned long Count)
{
	if(Source == NULL || Dest == NULL || Count == 0)
	{
		return;
	}

	unsigned long* SourceLongs = (unsigned long*)Source;
	unsigned long long Bits = 0;
	unsigned long long Input = 0;
	unsigned long long Output;
	const unsigned long long Mask = 0x3F;
	
	do
	{
		if(Bits < 6)
		{
			Bits += 26;
			Input = (Input << 32) | (unsigned long long)*SourceLongs;
			SourceLongs++;
			Output = (Input & (Mask << Bits)) >> Bits;
		}
		else
		{
			Bits -= 6;
			Output = (Input & (Mask << Bits)) >> Bits;
		}
		*Dest = (unsigned long)Output;
		Dest++;
		Count--;
	} while(Count);
	return;
}

void Expand4BitAdpcmBlock(void* Source, unsigned long* Dest, unsigned long Count)
{
	if(Source == NULL || Dest == NULL || Count == 0)
	{
		return;
	}

	unsigned long* SourceLongs = (unsigned long*)Source;
	unsigned long long Bits = 0;
	unsigned long long Input = 0;
	unsigned long long Output;
	const unsigned long long Mask = 0x0F;
	
	do
	{
		if(Bits < 4)
		{
			Bits += 28;
			Input = (Input << 32) | (unsigned long long)*SourceLongs;
			SourceLongs++;
			Output = (Input & (Mask << Bits)) >> Bits;
		}
		else
		{
			Bits -= 4;
			Output = (Input & (Mask << Bits)) >> Bits;
		}
		*Dest = (unsigned long)Output;
		Dest++;
		Count--;
	} while(Count);
	return;
}

void DecompressStereo6BitAdpcmBlock(S6BitAdpcmBlockHeader& Left, \
									S6BitAdpcmBlockHeader& Right, \
									unsigned long* Expanded, short* Output, \
									unsigned long SampleCount)
{
	// Sorry for this crappy mess
	// I should actually convert it to C++, but it works so what the heck

	unsigned long i;
	unsigned long Temp;

	__asm
	{
		MOV EDI,SampleCount
		SAR EDI,1
		MOV EAX,0
		MOV i,EAX
loop_start:
		CMP EDI,EAX
		JE loop_end

		; First sample
		MOV EBX,Expanded
		MOV EBX,DWORD PTR DS:[EBX+EAX*8]        ;  the next input byte
		MOV ESI,Left                            ;  the header from the file
		SUB EBX,1Fh
		MOV ECX,DWORD PTR DS:[ESI+4]            ;  the second dword in the header (1)
		MOV EDX,EBX
		SAR EDX,1Fh                             ;  FFFFFFFF if negative, 00000000 if positive
		MOV EAX,EBX
		XOR EBX,EDX
		SAR EAX,1Fh                             ;  FFFFFFFF if negative, 00000000 if positive
		SUB EBX,EDX
		MOV Temp,EBX                            ;  store the value into a local
		AND EAX,84h                             ;  either 84 (if negative) or 0 (if positive) I think
		MOV EBX,DWORD PTR DS:[EBX*4+Lookup1]    ;  some sort of lookup table
		XOR EDX,EDX
		ADD EBX,ECX                             ;  ecx is some sort of constant
		LEA EDI,DWORD PTR DS:[Lookup2]          ;  another sort of lookup table
		MOV EDX,EBX
		AND EBX,0FFFFFF00h                       ;  keep everything that's not the lowest byte
		AND EDX,0FFh                            ;  keep everything that is
		ADD EDI,EAX                             ;  either 84 or 0 is added onto the lookup table
		SAR EDX,3
		MOV ECX,EBX
		SAR ECX,8
		DEC EBX
		SAR EBX,1Fh                             ;  FFFFFFFF if negative, 00000000 if positive
		MOV EAX,DWORD PTR DS:[EDI+EDX*4]        ;  do lookup
		MOVD MM0,ECX
		MOVD MM1,EAX
		MOVD MM6,DWORD PTR DS:[ESI+4]           ;  the second dword in the header (1)
		PSLLQ MM1,MM0                           ;  64-bit left shift
		MOV EDX,DWORD PTR DS:[ESI+28h]          ;  a dword in the header (2)
		MOV ECX,0F6h
		MOVD MM4,ECX
		MOVD EAX,MM1
		PMADDWD MM6,MM4
		SAR EAX,0Ah
		XOR EBX,0FFFFFFFFh
		MOV EDI,Temp                            ;  retrieve the value from the local
		AND EAX,EBX                             ;  either the same value or 0
		MOV EBX,DWORD PTR DS:[ESI+20h]          ;  LastSample in the header (3)
		ADD EDX,EAX
		ADD EDX,EBX
		MOV EBX,DWORD PTR DS:[EDI*4+Lookup3]    ;  another lookup with the same index as above
		MOVD EDI,MM6
		MOV DWORD PTR DS:[ESI+28h],EAX          ;  set the dword in the header (2)
		ADD EBX,EDI
		SAR EBX,8
		MOV DWORD PTR DS:[ESI+20h],EDX          ;  set the LastSample in the header (3)
		CMP EBX,10Fh                            ;  it has to be greater than 010F and less than 0A00
		JGE SHORT ge_1
		MOV EBX,10Fh
ge_1:
		CMP EBX,0A00h
		JLE SHORT le_1
		MOV EBX,0A00h
le_1:
		MOV DWORD PTR DS:[ESI+4],EBX            ;  set the second dword in the header (1)
		MOV EAX,i                               ;  set the counter in eax
		MOV EBX,Output                          ;  set the output buffer in ebx
		MOV WORD PTR DS:[EBX+EAX*4],DX          ;  dx is the sample, put it in the output
		
		; Second sample
		MOV EBX,Expanded
		MOV EBX,DWORD PTR DS:[EBX+EAX*8+8]      ;  the next input byte
		MOV ESI,Left                            ;  the header from the file
		SUB EBX,1Fh
		MOV ECX,DWORD PTR DS:[ESI+4]            ;  the second dword in the header (1)
		MOV EDX,EBX
		SAR EDX,1Fh                             ;  FFFFFFFF if negative, 00000000 if positive
		MOV EAX,EBX
		XOR EBX,EDX
		SAR EAX,1Fh                             ;  FFFFFFFF if negative, 00000000 if positive
		SUB EBX,EDX
		MOV Temp,EBX                            ;  store the value into a local
		AND EAX,84h                             ;  either 84 (if negative) or 0 (if positive) I think
		MOV EBX,DWORD PTR DS:[EBX*4+Lookup1]    ;  some sort of lookup table
		XOR EDX,EDX
		ADD EBX,ECX                             ;  ecx is some sort of constant
		LEA EDI,DWORD PTR DS:[Lookup2]          ;  another sort of lookup table
		MOV EDX,EBX
		AND EBX,0FFFFFF00h                       ;  keep everything that's not the lowest byte
		AND EDX,0FFh                            ;  keep everything that is
		ADD EDI,EAX                             ;  either 84 or 0 is added onto the lookup table
		SAR EDX,3
		MOV ECX,EBX
		SAR ECX,8
		DEC EBX
		SAR EBX,1Fh                             ;  FFFFFFFF if negative, 00000000 if positive
		MOV EAX,DWORD PTR DS:[EDI+EDX*4]        ;  do lookup
		MOVD MM0,ECX
		MOVD MM1,EAX
		MOVD MM6,DWORD PTR DS:[ESI+4]           ;  the second dword in the header (1)
		PSLLQ MM1,MM0                           ;  64-bit left shift
		MOV EDX,DWORD PTR DS:[ESI+28h]          ;  a dword in the header (2)
		MOV ECX,0F6h
		MOVD MM4,ECX
		MOVD EAX,MM1
		PMADDWD MM6,MM4
		SAR EAX,0Ah
		XOR EBX,0FFFFFFFFh
		MOV EDI,Temp                            ;  retrieve the value from the local
		AND EAX,EBX                             ;  either the same value or 0
		MOV EBX,DWORD PTR DS:[ESI+20h]          ;  LastSample in the header (3)
		ADD EDX,EAX
		ADD EDX,EBX
		MOV EBX,DWORD PTR DS:[EDI*4+Lookup3]    ;  another lookup with the same index as above
		MOVD EDI,MM6
		MOV DWORD PTR DS:[ESI+28h],EAX          ;  set the dword in the header (2)
		ADD EBX,EDI
		SAR EBX,8
		MOV DWORD PTR DS:[ESI+20h],EDX          ;  set the LastSample in the header (3)
		CMP EBX,10Fh                            ;  it has to be greater than 010F and less than 0A00
		JGE SHORT ge_2
		MOV EBX,10Fh
ge_2:
		CMP EBX,0A00h
		JLE SHORT le_2
		MOV EBX,0A00h
le_2:
		MOV DWORD PTR DS:[ESI+4],EBX            ;  set the second dword in the header (1)
		MOV EAX,i                               ;  set the counter in eax
		MOV EBX,Output                          ;  set the output buffer in ebx
		MOV WORD PTR DS:[EBX+EAX*4+2],DX          ;  dx is the sample, put it in the output
		;MOV WORD PTR DS:[EBX+EAX*4+4],DX

		; Third sample
		MOV EBX,Expanded
		MOV EBX,DWORD PTR DS:[EBX+EAX*8+10h]    ;  the next input byte
		MOV ESI,Left                            ;  the header from the file
		SUB EBX,1Fh
		MOV ECX,DWORD PTR DS:[ESI+4]            ;  the second dword in the header (1)
		MOV EDX,EBX
		SAR EDX,1Fh                             ;  FFFFFFFF if negative, 00000000 if positive
		MOV EAX,EBX
		XOR EBX,EDX
		SAR EAX,1Fh                             ;  FFFFFFFF if negative, 00000000 if positive
		SUB EBX,EDX
		MOV Temp,EBX                            ;  store the value into a local
		AND EAX,84h                             ;  either 84 (if negative) or 0 (if positive) I think
		MOV EBX,DWORD PTR DS:[EBX*4+Lookup1]    ;  some sort of lookup table
		XOR EDX,EDX
		ADD EBX,ECX                             ;  ecx is some sort of constant
		LEA EDI,DWORD PTR DS:[Lookup2]          ;  another sort of lookup table
		MOV EDX,EBX
		AND EBX,0FFFFFF00h                       ;  keep everything that's not the lowest byte
		AND EDX,0FFh                            ;  keep everything that is
		ADD EDI,EAX                             ;  either 84 or 0 is added onto the lookup table
		SAR EDX,3
		MOV ECX,EBX
		SAR ECX,8
		DEC EBX
		SAR EBX,1Fh                             ;  FFFFFFFF if negative, 00000000 if positive
		MOV EAX,DWORD PTR DS:[EDI+EDX*4]        ;  do lookup
		MOVD MM0,ECX
		MOVD MM1,EAX
		MOVD MM6,DWORD PTR DS:[ESI+4]           ;  the second dword in the header (1)
		PSLLQ MM1,MM0                           ;  64-bit left shift
		MOV EDX,DWORD PTR DS:[ESI+28h]          ;  a dword in the header (2)
		MOV ECX,0F6h
		MOVD MM4,ECX
		MOVD EAX,MM1
		PMADDWD MM6,MM4
		SAR EAX,0Ah
		XOR EBX,0FFFFFFFFh
		MOV EDI,Temp                            ;  retrieve the value from the local
		AND EAX,EBX                             ;  either the same value or 0
		MOV EBX,DWORD PTR DS:[ESI+20h]          ;  LastSample in the header (3)
		ADD EDX,EAX
		ADD EDX,EBX
		MOV EBX,DWORD PTR DS:[EDI*4+Lookup3]    ;  another lookup with the same index as above
		MOVD EDI,MM6
		MOV DWORD PTR DS:[ESI+28h],EAX          ;  set the dword in the header (2)
		ADD EBX,EDI
		SAR EBX,8
		MOV DWORD PTR DS:[ESI+20h],EDX          ;  set the LastSample in the header (3)
		CMP EBX,10Fh                            ;  it has to be greater than 010F and less than 0A00
		JGE SHORT ge_3
		MOV EBX,10Fh
ge_3:
		CMP EBX,0A00h
		JLE SHORT le_3
		MOV EBX,0A00h
le_3:
		MOV DWORD PTR DS:[ESI+4],EBX            ;  set the second dword in the header (1)
		MOV EAX,i                               ;  set the counter in eax
		MOV EBX,Output                          ;  set the output buffer in ebx
		MOV WORD PTR DS:[EBX+EAX*4+4],DX          ;  dx is the sample, put it in the output
		;MOV WORD PTR DS:[EBX+EAX*4+8],DX

		; Fourth sample
		MOV EBX,Expanded
		MOV EBX,DWORD PTR DS:[EBX+EAX*8+18h]    ;  the next input byte
		MOV ESI,Left                            ;  the header from the file
		SUB EBX,1Fh
		MOV ECX,DWORD PTR DS:[ESI+4]            ;  the second dword in the header (1)
		MOV EDX,EBX
		SAR EDX,1Fh                             ;  FFFFFFFF if negative, 00000000 if positive
		MOV EAX,EBX
		XOR EBX,EDX
		SAR EAX,1Fh                             ;  FFFFFFFF if negative, 00000000 if positive
		SUB EBX,EDX
		MOV Temp,EBX                            ;  store the value into a local
		AND EAX,84h                             ;  either 84 (if negative) or 0 (if positive) I think
		MOV EBX,DWORD PTR DS:[EBX*4+Lookup1]    ;  some sort of lookup table
		XOR EDX,EDX
		ADD EBX,ECX                             ;  ecx is some sort of constant
		LEA EDI,DWORD PTR DS:[Lookup2]          ;  another sort of lookup table
		MOV EDX,EBX
		AND EBX,0FFFFFF00h                       ;  keep everything that's not the lowest byte
		AND EDX,0FFh                            ;  keep everything that is
		ADD EDI,EAX                             ;  either 84 or 0 is added onto the lookup table
		SAR EDX,3
		MOV ECX,EBX
		SAR ECX,8
		DEC EBX
		SAR EBX,1Fh                             ;  FFFFFFFF if negative, 00000000 if positive
		MOV EAX,DWORD PTR DS:[EDI+EDX*4]        ;  do lookup
		MOVD MM0,ECX
		MOVD MM1,EAX
		MOVD MM6,DWORD PTR DS:[ESI+4]           ;  the second dword in the header (1)
		PSLLQ MM1,MM0                           ;  64-bit left shift
		MOV EDX,DWORD PTR DS:[ESI+28h]          ;  a dword in the header (2)
		MOV ECX,0F6h
		MOVD MM4,ECX
		MOVD EAX,MM1
		PMADDWD MM6,MM4
		SAR EAX,0Ah
		XOR EBX,0FFFFFFFFh
		MOV EDI,Temp                            ;  retrieve the value from the local
		AND EAX,EBX                             ;  either the same value or 0
		MOV EBX,DWORD PTR DS:[ESI+20h]          ;  LastSample in the header (3)
		ADD EDX,EAX
		ADD EDX,EBX
		MOV EBX,DWORD PTR DS:[EDI*4+Lookup3]    ;  another lookup with the same index as above
		MOVD EDI,MM6
		MOV DWORD PTR DS:[ESI+28h],EAX          ;  set the dword in the header (2)
		ADD EBX,EDI
		SAR EBX,8
		MOV DWORD PTR DS:[ESI+20h],EDX          ;  set the LastSample in the header (3)
		CMP EBX,10Fh                            ;  it has to be greater than 010F and less than 0A00
		JGE SHORT ge_4
		MOV EBX,10Fh
ge_4:
		CMP EBX,0A00h
		JLE SHORT le_4
		MOV EBX,0A00h
le_4:
		MOV DWORD PTR DS:[ESI+4],EBX            ;  set the second dword in the header (1)
		MOV EAX,i                               ;  set the counter in eax
		MOV EBX,Output                          ;  set the output buffer in ebx
		MOV WORD PTR DS:[EBX+EAX*4+6],DX        ;  dx is the sample, put it in the output
		;MOV WORD PTR DS:[EBX+EAX*4+0Ch],DX

		; Fifth sample
		MOV EBX,Expanded
		MOV EBX,DWORD PTR DS:[EBX+EAX*8+4]      ;  the next input byte
		MOV ESI,Right                           ;  the header from the file
		SUB EBX,1Fh
		MOV ECX,DWORD PTR DS:[ESI+4]            ;  the second dword in the header (1)
		MOV EDX,EBX
		SAR EDX,1Fh                             ;  FFFFFFFF if negative, 00000000 if positive
		MOV EAX,EBX
		XOR EBX,EDX
		SAR EAX,1Fh                             ;  FFFFFFFF if negative, 00000000 if positive
		SUB EBX,EDX
		MOV Temp,EBX                            ;  store the value into a local
		AND EAX,84h                             ;  either 84 (if negative) or 0 (if positive) I think
		MOV EBX,DWORD PTR DS:[EBX*4+Lookup1]    ;  some sort of lookup table
		XOR EDX,EDX
		ADD EBX,ECX                             ;  ecx is some sort of constant
		LEA EDI,DWORD PTR DS:[Lookup2]          ;  another sort of lookup table
		MOV EDX,EBX
		AND EBX,0FFFFFF00h                       ;  keep everything that's not the lowest byte
		AND EDX,0FFh                            ;  keep everything that is
		ADD EDI,EAX                             ;  either 84 or 0 is added onto the lookup table
		SAR EDX,3
		MOV ECX,EBX
		SAR ECX,8
		DEC EBX
		SAR EBX,1Fh                             ;  FFFFFFFF if negative, 00000000 if positive
		MOV EAX,DWORD PTR DS:[EDI+EDX*4]        ;  do lookup
		MOVD MM0,ECX
		MOVD MM1,EAX
		MOVD MM6,DWORD PTR DS:[ESI+4]           ;  the second dword in the header (1)
		PSLLQ MM1,MM0                           ;  64-bit left shift
		MOV EDX,DWORD PTR DS:[ESI+28h]          ;  a dword in the header (2)
		MOV ECX,0F6h
		MOVD MM4,ECX
		MOVD EAX,MM1
		PMADDWD MM6,MM4
		SAR EAX,0Ah
		XOR EBX,0FFFFFFFFh
		MOV EDI,Temp                            ;  retrieve the value from the local
		AND EAX,EBX                             ;  either the same value or 0
		MOV EBX,DWORD PTR DS:[ESI+20h]          ;  LastSample in the header (3)
		ADD EDX,EAX
		ADD EDX,EBX
		MOV EBX,DWORD PTR DS:[EDI*4+Lookup3]    ;  another lookup with the same index as above
		MOVD EDI,MM6
		MOV DWORD PTR DS:[ESI+28h],EAX          ;  set the dword in the header (2)
		ADD EBX,EDI
		SAR EBX,8
		MOV DWORD PTR DS:[ESI+20h],EDX          ;  set the LastSample in the header (3)
		CMP EBX,10Fh                            ;  it has to be greater than 010F and less than 0A00
		JGE SHORT ge_5
		MOV EBX,10Fh
ge_5:
		CMP EBX,0A00h
		JLE SHORT le_5
		MOV EBX,0A00h
le_5:
		MOV DWORD PTR DS:[ESI+4],EBX            ;  set the second dword in the header (1)
		MOV EAX,i                               ;  set the counter in eax
		MOV EBX,Output                          ;  set the output buffer in ebx
		MOV WORD PTR DS:[EBX+EAX*4+8],DX          ;  dx is the sample, put it in the output
		;MOV WORD PTR DS:[EBX+EAX*4+2],DX

		; Sixth sample
		MOV EBX,Expanded
		MOV EBX,DWORD PTR DS:[EBX+EAX*8+0Ch]     ;  the next input byte
		MOV ESI,Right                           ;  the header from the file
		SUB EBX,1Fh
		MOV ECX,DWORD PTR DS:[ESI+4]            ;  the second dword in the header (1)
		MOV EDX,EBX
		SAR EDX,1Fh                             ;  FFFFFFFF if negative, 00000000 if positive
		MOV EAX,EBX
		XOR EBX,EDX
		SAR EAX,1Fh                             ;  FFFFFFFF if negative, 00000000 if positive
		SUB EBX,EDX
		MOV Temp,EBX                            ;  store the value into a local
		AND EAX,84h                             ;  either 84 (if negative) or 0 (if positive) I think
		MOV EBX,DWORD PTR DS:[EBX*4+Lookup1]    ;  some sort of lookup table
		XOR EDX,EDX
		ADD EBX,ECX                             ;  ecx is some sort of constant
		LEA EDI,DWORD PTR DS:[Lookup2]          ;  another sort of lookup table
		MOV EDX,EBX
		AND EBX,0FFFFFF00h                       ;  keep everything that's not the lowest byte
		AND EDX,0FFh                            ;  keep everything that is
		ADD EDI,EAX                             ;  either 84 or 0 is added onto the lookup table
		SAR EDX,3
		MOV ECX,EBX
		SAR ECX,8
		DEC EBX
		SAR EBX,1Fh                             ;  FFFFFFFF if negative, 00000000 if positive
		MOV EAX,DWORD PTR DS:[EDI+EDX*4]        ;  do lookup
		MOVD MM0,ECX
		MOVD MM1,EAX
		MOVD MM6,DWORD PTR DS:[ESI+4]           ;  the second dword in the header (1)
		PSLLQ MM1,MM0                           ;  64-bit left shift
		MOV EDX,DWORD PTR DS:[ESI+28h]          ;  a dword in the header (2)
		MOV ECX,0F6h
		MOVD MM4,ECX
		MOVD EAX,MM1
		PMADDWD MM6,MM4
		SAR EAX,0Ah
		XOR EBX,0FFFFFFFFh
		MOV EDI,Temp                            ;  retrieve the value from the local
		AND EAX,EBX                             ;  either the same value or 0
		MOV EBX,DWORD PTR DS:[ESI+20h]          ;  LastSample in the header (3)
		ADD EDX,EAX
		ADD EDX,EBX
		MOV EBX,DWORD PTR DS:[EDI*4+Lookup3]    ;  another lookup with the same index as above
		MOVD EDI,MM6
		MOV DWORD PTR DS:[ESI+28h],EAX          ;  set the dword in the header (2)
		ADD EBX,EDI
		SAR EBX,8
		MOV DWORD PTR DS:[ESI+20h],EDX          ;  set the LastSample in the header (3)
		CMP EBX,10Fh                            ;  it has to be greater than 010F and less than 0A00
		JGE SHORT ge_6
		MOV EBX,10Fh
ge_6:
		CMP EBX,0A00h
		JLE SHORT le_6
		MOV EBX,0A00h
le_6:
		MOV DWORD PTR DS:[ESI+4],EBX            ;  set the second dword in the header (1)
		MOV EAX,i                               ;  set the counter in eax
		MOV EBX,Output                          ;  set the output buffer in ebx
		MOV WORD PTR DS:[EBX+EAX*4+0Ah],DX       ;  dx is the sample, put it in the output
		;MOV WORD PTR DS:[EBX+EAX*4+6],DX

		; Seventh sample
		MOV EBX,Expanded
		MOV EBX,DWORD PTR DS:[EBX+EAX*8+14h]    ;  the next input byte
		MOV ESI,Right                           ;  the header from the file
		SUB EBX,1Fh
		MOV ECX,DWORD PTR DS:[ESI+4]            ;  the second dword in the header (1)
		MOV EDX,EBX
		SAR EDX,1Fh                             ;  FFFFFFFF if negative, 00000000 if positive
		MOV EAX,EBX
		XOR EBX,EDX
		SAR EAX,1Fh                             ;  FFFFFFFF if negative, 00000000 if positive
		SUB EBX,EDX
		MOV Temp,EBX                            ;  store the value into a local
		AND EAX,84h                             ;  either 84 (if negative) or 0 (if positive) I think
		MOV EBX,DWORD PTR DS:[EBX*4+Lookup1]    ;  some sort of lookup table
		XOR EDX,EDX
		ADD EBX,ECX                             ;  ecx is some sort of constant
		LEA EDI,DWORD PTR DS:[Lookup2]          ;  another sort of lookup table
		MOV EDX,EBX
		AND EBX,0FFFFFF00h                       ;  keep everything that's not the lowest byte
		AND EDX,0FFh                            ;  keep everything that is
		ADD EDI,EAX                             ;  either 84 or 0 is added onto the lookup table
		SAR EDX,3
		MOV ECX,EBX
		SAR ECX,8
		DEC EBX
		SAR EBX,1Fh                             ;  FFFFFFFF if negative, 00000000 if positive
		MOV EAX,DWORD PTR DS:[EDI+EDX*4]        ;  do lookup
		MOVD MM0,ECX
		MOVD MM1,EAX
		MOVD MM6,DWORD PTR DS:[ESI+4]           ;  the second dword in the header (1)
		PSLLQ MM1,MM0                           ;  64-bit left shift
		MOV EDX,DWORD PTR DS:[ESI+28h]          ;  a dword in the header (2)
		MOV ECX,0F6h
		MOVD MM4,ECX
		MOVD EAX,MM1
		PMADDWD MM6,MM4
		SAR EAX,0Ah
		XOR EBX,0FFFFFFFFh
		MOV EDI,Temp                            ;  retrieve the value from the local
		AND EAX,EBX                             ;  either the same value or 0
		MOV EBX,DWORD PTR DS:[ESI+20h]          ;  LastSample in the header (3)
		ADD EDX,EAX
		ADD EDX,EBX
		MOV EBX,DWORD PTR DS:[EDI*4+Lookup3]    ;  another lookup with the same index as above
		MOVD EDI,MM6
		MOV DWORD PTR DS:[ESI+28h],EAX          ;  set the dword in the header (2)
		ADD EBX,EDI
		SAR EBX,8
		MOV DWORD PTR DS:[ESI+20h],EDX          ;  set the LastSample in the header (3)
		CMP EBX,10Fh                            ;  it has to be greater than 010F and less than 0A00
		JGE SHORT ge_7
		MOV EBX,10Fh
ge_7:
		CMP EBX,0A00h
		JLE SHORT le_7
		MOV EBX,0A00h
le_7:
		MOV DWORD PTR DS:[ESI+4],EBX            ;  set the second dword in the header (1)
		MOV EAX,i                               ;  set the counter in eax
		MOV EBX,Output                          ;  set the output buffer in ebx
		MOV WORD PTR DS:[EBX+EAX*4+0Ch],DX       ;  dx is the sample, put it in the output
		;MOV WORD PTR DS:[EBX+EAX*4+0Ah],DX

		; Eigth sample
		MOV EBX,Expanded
		MOV EBX,DWORD PTR DS:[EBX+EAX*8+1Ch]    ;  the next input byte
		MOV ESI,Right                           ;  the header from the file
		SUB EBX,1Fh
		MOV ECX,DWORD PTR DS:[ESI+4]            ;  the second dword in the header (1)
		MOV EDX,EBX
		SAR EDX,1Fh                             ;  FFFFFFFF if negative, 00000000 if positive
		MOV EAX,EBX
		XOR EBX,EDX
		SAR EAX,1Fh                             ;  FFFFFFFF if negative, 00000000 if positive
		SUB EBX,EDX
		MOV Temp,EBX                            ;  store the value into a local
		AND EAX,84h                             ;  either 84 (if negative) or 0 (if positive) I think
		MOV EBX,DWORD PTR DS:[EBX*4+Lookup1]    ;  some sort of lookup table
		XOR EDX,EDX
		ADD EBX,ECX                             ;  ecx is some sort of constant
		LEA EDI,DWORD PTR DS:[Lookup2]          ;  another sort of lookup table
		MOV EDX,EBX
		AND EBX,0FFFFFF00h                       ;  keep everything that's not the lowest byte
		AND EDX,0FFh                            ;  keep everything that is
		ADD EDI,EAX                             ;  either 84 or 0 is added onto the lookup table
		SAR EDX,3
		MOV ECX,EBX
		SAR ECX,8
		DEC EBX
		SAR EBX,1Fh                             ;  FFFFFFFF if negative, 00000000 if positive
		MOV EAX,DWORD PTR DS:[EDI+EDX*4]        ;  do lookup
		MOVD MM0,ECX
		MOVD MM1,EAX
		MOVD MM6,DWORD PTR DS:[ESI+4]           ;  the second dword in the header (1)
		PSLLQ MM1,MM0                           ;  64-bit left shift
		MOV EDX,DWORD PTR DS:[ESI+28h]          ;  a dword in the header (2)
		MOV ECX,0F6h
		MOVD MM4,ECX
		MOVD EAX,MM1
		PMADDWD MM6,MM4
		SAR EAX,0Ah
		XOR EBX,0FFFFFFFFh
		MOV EDI,Temp                            ;  retrieve the value from the local
		AND EAX,EBX                             ;  either the same value or 0
		MOV EBX,DWORD PTR DS:[ESI+20h]          ;  LastSample in the header (3)
		ADD EDX,EAX
		ADD EDX,EBX
		MOV EBX,DWORD PTR DS:[EDI*4+Lookup3]    ;  another lookup with the same index as above
		MOVD EDI,MM6
		MOV DWORD PTR DS:[ESI+28h],EAX          ;  set the dword in the header (2)
		ADD EBX,EDI
		SAR EBX,8
		MOV DWORD PTR DS:[ESI+20h],EDX          ;  set the LastSample in the header (3)
		CMP EBX,10Fh                            ;  it has to be greater than 010F and less than 0A00
		JGE SHORT ge_8
		MOV EBX,10Fh
ge_8:
		CMP EBX,0A00h
		JLE SHORT le_8
		MOV EBX,0A00h
le_8:
		MOV DWORD PTR DS:[ESI+4],EBX            ;  set the second dword in the header (1)
		MOV EAX,i                               ;  set the counter in eax
		MOV EBX,Output                          ;  set the output buffer in ebx
		MOV WORD PTR DS:[EBX+EAX*4+0Eh],DX       ;  dx is the sample, put it in the output
		;MOV WORD PTR DS:[EBX+EAX*4+0Eh],DX

		ADD EAX,4                               ;  increment counter
		MOV EDI,SampleCount
		MOV i,EAX
		SAR EDI,1
		JMP loop_start
loop_end:
		EMMS

		MOV ECX,0
		MOV EBX,SampleCount
		MOV ESI,Output
		MOV EDX,Output
smallloop_start:
		MOVQ MM0,QWORD PTR DS:[ESI]
		MOVQ MM1,QWORD PTR DS:[ESI+8]
		MOVQ MM2,MM0
		PADDSW MM0,MM1
		ADD ESI,10h
		PSUBSW MM2,MM1
		MOVQ MM3,MM0
		PUNPCKLWD MM3,MM2
		ADD EDX,10h
		PUNPCKHWD MM0,MM2
		ADD ECX,8
		MOVQ QWORD PTR DS:[EDX-10h],MM3
		MOVQ QWORD PTR DS:[EDX-8h],MM0
		CMP ECX,EBX
		JL SHORT smallloop_start
		EMMS
	};
	return;
}

void DecompressMono6BitAdpcmBlock(S6BitAdpcmBlockHeader& Header, \
								  unsigned long* Expanded, short* Output, \
								  unsigned long SampleCount)
{
	// Sorry for this crappy mess
	// I should actually convert it to C++, but it works so what the heck

	unsigned long i;
	unsigned long Temp;

	__asm
	{
;01B62463  |. 8945 E0        MOV DWORD PTR SS:[EBP-20],EAX            ;  header
;01B62466  |. 894D E4        MOV DWORD PTR SS:[EBP-1C],ECX            ;  input
;01B62469  |. 897D DC        MOV DWORD PTR SS:[EBP-24],EDI            ;  output

		MOV EDI,SampleCount
		MOV EAX,0
loop_start:
		MOV EBX,Expanded
		MOV ESI,Header
		CMP EAX,EDI
		JGE loop_end
		MOV EBX,DWORD PTR DS:[EBX+EAX*4]
		MOV i,EAX
		SUB EBX,1Fh
		MOV ECX,DWORD PTR DS:[ESI+4]
		MOV EDX,EBX
		SAR EDX,1Fh
		XOR EBX,EDX
		MOV EAX,EDX
		SUB EBX,EDX
		AND EAX,84h
		MOV Temp,EBX
		LEA EDI,Lookup2
		MOV EBX,DWORD PTR DS:[EBX*4+Lookup1]
		ADD EDI,EAX
		ADD EBX,ECX
		MOV EDX,EBX
		AND EBX,0FFFFFF00h
		AND EDX,0FFh
		MOV ECX,EBX
		SAR EDX,3
		DEC EBX
		SAR ECX,8
		SAR EBX,1Fh
		MOV EAX,DWORD PTR DS:[EDI+EDX*4]
		MOVD MM0,ECX
		MOVD MM1,EAX
		MOVD MM6,DWORD PTR DS:[ESI+4]
		PSLLQ MM1,MM0
		MOV EDX,DWORD PTR DS:[ESI+28h]
		MOV ECX,0F6h
		MOVD MM4,ECX
		MOVD EAX,MM1
		PMADDWD MM6,MM4
		SAR EAX,0Ah
		XOR EBX,0FFFFFFFFh
		MOV EDI,Temp
		AND EAX,EBX
		MOV EBX,DWORD PTR DS:[ESI+20h]
		ADD EDX,EAX
		ADD EDX,EBX
		MOV EBX,DWORD PTR DS:[EDI*4+Lookup3]
		MOVD EDI,MM6
		MOV DWORD PTR DS:[ESI+28h],EAX
		ADD EBX,EDI
		SAR EBX,8
		MOV DWORD PTR DS:[ESI+20h],EDX
		MOV ECX,Expanded
		MOV EAX,i
		CMP EBX,10Fh
		JGE SHORT ge_1
		MOV EBX,10Fh
ge_1:
		CMP EBX,0A00h
		JLE SHORT le_1
		MOV EBX,0A00h
le_1:
		MOV EDI,Output
		MOV DWORD PTR DS:[ESI+4],EBX
		MOV EBX,DWORD PTR DS:[ECX+EAX*4+4]
		MOV ECX,DWORD PTR DS:[ESI+4]
		SUB EBX,1Fh
		MOV DWORD PTR DS:[EDI+EAX*2],EDX
		MOV EDX,EBX
		SAR EDX,1Fh
		MOV EAX,EBX
		XOR EBX,EDX
		SAR EAX,1Fh
		SUB EBX,EDX
		MOV Temp,EBX
		AND EAX,84h
		MOV EBX,DWORD PTR DS:[EBX*4+Lookup1]
		ADD EBX,ECX
		LEA EDI,DWORD PTR DS:[Lookup2]
		MOV EDX,EBX
		AND EBX,0FFFFFF00h
		AND EDX,0FFh
		ADD EDI,EAX
		SAR EDX,3
		MOV ECX,EBX
		SAR ECX,8
		DEC EBX
		SAR EBX,1Fh
		MOV EAX,DWORD PTR DS:[EDI+EDX*4]
		MOVD MM0,ECX
		MOVD MM1,EAX
		MOVD MM6,DWORD PTR DS:[ESI+4]
		PSLLQ MM1,MM0
		MOV EDX,DWORD PTR DS:[ESI+28h]
		MOV ECX,0F6h
		MOVD MM4,ECX
		MOVD EAX,MM1
		PMADDWD MM6,MM4
		SAR EAX,0Ah
		XOR EBX,0FFFFFFFFh
		MOV EDI,Temp
		AND EAX,EBX
		MOV EBX,DWORD PTR DS:[ESI+20h]
		ADD EDX,EAX
		ADD EDX,EBX
		MOV EBX,DWORD PTR DS:[EDI*4+Lookup3]
		MOVD EDI,MM6
		MOV DWORD PTR DS:[ESI+28h],EAX
		ADD EBX,EDI
		SAR EBX,8
		MOV DWORD PTR DS:[ESI+20h],EDX
		MOV EAX,i
		MOV EDI,Output
		CMP EBX,10Fh
		JGE SHORT ge_2
		MOV EBX,10Fh
ge_2:
		CMP EBX,0A00h
		JLE SHORT le_2
		MOV EBX,0A00h
le_2:
		MOV WORD PTR DS:[EDI+EAX*2+2],DX
		MOV DWORD PTR DS:[ESI+4],EBX
		ADD EAX,2
		MOV EDI,SampleCount
		JMP loop_start
loop_end:
		EMMS
	};
	return;
}

void DecompressStereo4BitAdpcmBlock(S4BitAdpcmBlockHeader& Left, \
									S4BitAdpcmBlockHeader& Right, \
									unsigned long* Expanded, short* Output, \
									unsigned long SampleCount)
{
	// Sorry for this crappy mess
	// I should actually convert it to C++, but it works so what the heck

	short* Local2C=Output;
	S4BitAdpcmBlockHeader* Local30=&Left;
	S4BitAdpcmBlockHeader* Local28=&Right;
	long* Local1C=(long*)Expanded;
	long Local24=0;
	long Local20=0;
	long Local0C=0;
	long Local3C;
	long Local38;
	long Local34;
	unsigned long Local10=SampleCount;

	const unsigned long long Constant17747A0=0x000200FE00FF0C00;
	const unsigned long long Constant17747A8=0x7800780077FF77FF;
	const unsigned long long Constant17747B0=0x0001000100010001;
	const unsigned long long Constant17747B8=0x0000000004000000;
	const unsigned long long Constant17747C0=0x000000000000FFFF;
	const unsigned long long Constant17747D0=0x00FF00FF00FF00FF;
	const unsigned long long Constant17747D8=0xF000F000F000F000;
	const unsigned long long Constant17747E0=0x0800080008000800;
	const unsigned long long Constant17747E8=0xFFFFFFFFFFFFFFFF;
	const unsigned long Constant17747C8=0x00FE00FF;

	__asm
	{
upperloop_start: ; 0173CCDA
		MOV EBX,0
		MOV Local24,0

		; Left samples
innerloop1_start: ; 0173CCE6
		MOV EDI,Local1C
		MOV EBX,DWORD PTR DS:[EDI+EBX*8]
		MOV EAX,Local30
		MOVD MM0,DWORD PTR DS:[EAX+10h]
		MOVD MM6,DWORD PTR DS:[EAX+20h]
		MOVQ MM2,QWORD PTR DS:[EAX+18h]
		MOVQ MM4,QWORD PTR DS:[EAX+28h]
		PMADDWD MM0,MM6
		MOV ESI,Local24
		SUB EBX,7
		MOV ECX,DWORD PTR DS:[EAX+4]
		MOV EDX,EBX
		MOV DWORD PTR DS:[EDI+ESI*8],EBX
		MOV EDI,EBX
		SAR EDX,1Fh
		MOV SI,WORD PTR DS:[EAX+2Eh]
		XOR EBX,EDX
		MOVQ MM3,QWORD PTR DS:[EAX+28h]
		PSRAD MM0,0Ah
		SUB EBX,EDX
		PMADDWD MM2,MM3
		MOV Local3C,EBX
		MOV EAX,EDI
		MOV EBX,DWORD PTR DS:[EBX*4+Lookup1774338]
		XOR EDX,EDX
		MOVD Local38,MM0
		MOVQ MM1,MM2
		ADD EBX,ECX
		PUNPCKHDQ MM1,MM2
		MOV EDX,EBX
		PADDD MM1,MM2
		AND EBX,0FFFFFF00h
		PSRAD MM1,0Ah
		SAR EAX,1Fh
		SUB EDX,EBX
		LEA EDI,Lookup1774648
		AND EAX,84h
		SAR EDX,3
		ADD EDI,EAX
		MOVD Local20,MM1
		PSLLQ MM4,10h
		MOV ECX,EBX
		DEC EBX
		SAR ECX,8
		MOV EDX,DWORD PTR DS:[EDI+EDX*4]
		SAR EBX,1Fh
		MOV EAX,Local30
		SHL EDX,CL
		SAR EDX,0Ah
		XOR EBX,0FFFFFFFFh
		MOV ECX,Local20
		AND EDX,EBX
		MOV EBX,Local38
		ADD ECX,EDX
		ADD ECX,EBX
		MOV Local34,EDX
		MOVD MM5,ECX
		PSLLQ MM6,10h
		MOVD MM0,Local34
		PSLLD MM5,10h
		MOVQ MM2,QWORD PTR DS:[EAX+8]
		PSLLD MM0,10h
		MOVQ MM3,Constant17747A0
		PSRLD MM5,10h
		PSRLD MM0,10h
		POR MM5,MM6
		POR MM4,MM0
		PADDW MM1,MM0
		MOVD DWORD PTR DS:[EAX+20h],MM5
		PACKSSDW MM2,MM5
		MOVQ QWORD PTR DS:[EAX+28h],MM4
		PSLLQ MM2,10h
		MOV WORD PTR DS:[EAX+30h],SI
		MOV ESI,Local0C
		MOV EBX,Local2C
		MOV WORD PTR DS:[EBX+ESI*2],CX ; Output
		MOVD EBX,MM1
		PSLLD MM1,10h
		AND EBX,0FFFFh
		JE jump_0173CF42
		MOVQ MM0,QWORD PTR DS:[EAX+10h]
		PSRAD MM1,1Fh
		POR MM1,Constant17747B0
		PADDSW MM4,MM2
		PSRAW MM4,0Fh
		MOVQ MM5,MM3
		POR MM4,Constant17747B0
		PSLLD MM0,10h
		MOVQ MM7,Constant17747A8
		PSRLQ MM4,10h
		PMULLW MM4,MM1
		MOV ESI,Local30
		MOVQ MM1,QWORD PTR DS:[EAX+10h]
		PSRLQ MM5,20h
		PSLLQ MM1,20h
		MOV EAX,7CFF0000h
		MOVQ MM2,MM4
		PSRLQ MM1,30h
		PAND MM4,Constant17747C0
		POR MM4,MM0
		MOV EBX,Local3C
		MOVD MM6,DWORD PTR DS:[ESI+4]
		PMADDWD MM3,MM4
		MOV ECX,0F6h
		MOV EBX,DWORD PTR DS:[EBX*4+Lookup1774378]
		MOVD MM4,ECX
		PMADDWD MM6,MM4
		PSRLD MM3,8
		MOVQ MM4,MM3
		PSLLD MM3,2
		MOVD EDI,MM6
		PADDSW MM3,MM7
		PSUBSW MM3,MM7
		PSRLQ MM7,20h
		MOVQ MM0,QWORD PTR DS:[ESI+28h]
		PSUBSW MM3,MM7
		PSLLQ MM0,30h
		PADDSW MM3,MM7
		MOVQ MM6,MM0
		PAND MM3,Constant17747C0
		PSRLQ MM0,20h
		POR MM3,Constant17747B8
		POR MM0,MM6
		PMULLW MM3,MM2
		ADD EBX,EDI
		PSRAD MM0,20h
		SAR EBX,8
		MOVQ MM2,MM3
		PAND MM0,Constant17747D8
		PSRLQ MM2,10h
		MOVQ MM6,QWORD PTR DS:[ESI+2Ah]
		PSUBW MM2,MM3
		PCMPGTW MM6,Constant17747E8
		PSLLD MM2,10h
		PANDN MM6,Constant17747E8
		POR MM1,MM2
		PADDUSW MM6,Constant17747B0
		PMADDWD MM1,MM5
		CMP EBX,10Fh
		JGE SHORT ge_1
		MOV EBX,10Fh
ge_1: ; 0173CEF1
		CMP EBX,0A00h
		JLE SHORT le_1
		MOV EBX,0A00h
le_1: ; 0173CEFE
		MOV DWORD PTR DS:[ESI+4],EBX
		MOVQ MM2,Constant17747D0
		PSRAD MM1,8
		POR MM0,Constant17747E0
		PSLLD MM1,10h
		PMULLW MM2,QWORD PTR DS:[ESI+18h]
		PSLLD MM4,10h
		MOV ECX,3C0h
		MOV EBX,83000000h
		PMULLW MM0,MM6
		PXOR MM6,MM6
		PSRLD MM4,10h
		PADDSW MM2,MM0
		POR MM1,MM4
		PSRAW MM2,8
		JMP jump_0173D005
jump_0173CF42:
		MOVQ MM1,QWORD PTR DS:[EAX+10h]
		MOVD MM3,Constant17747C8
		MOVQ MM2,MM1
		PMULLW MM1,MM3
		MOV ESI,Local30
		MOV EAX,7CFF0000h
		PMULHW MM2,MM3
		PUNPCKLWD MM1,MM2
		PSRAD MM1,8
		PACKSSDW MM1,MM3
		MOVQ MM0,QWORD PTR DS:[ESI+28h]
		MOVD MM7,DWORD PTR DS:[ESI+4]
		PSLLQ MM0,30h
		MOV ECX,0F6h
		MOV EBX,Local3C
		MOVD MM4,ECX
		MOVQ MM6,MM0
		PSRLQ MM0,20h
		MOV EBX,DWORD PTR DS:[EBX*4+Lookup1774378]
		POR MM0,MM6
		MOVQ MM6,QWORD PTR DS:[ESI+2Ah]
		PSRAD MM0,20h
		PAND MM0,Constant17747D8
		PMADDWD MM7,MM4
		MOVQ MM2,Constant17747D0
		PCMPGTW MM6,Constant17747E8
		PANDN MM6,Constant17747E8
		PMULLW MM2,QWORD PTR DS:[ESI+18h]
		POR MM0,Constant17747E0
		PADDUSW MM6,Constant17747B0
		MOVD EDI,MM7
		PMULLW MM0,MM6
		ADD EBX,EDI
		SAR EBX,8
		CMP EBX,10Fh
		JGE SHORT ge_2
		MOV EBX,10Fh
ge_2:
		PADDSW MM2,MM0
		CMP EBX,0A00h
		PSRAW MM2,8
		JLE SHORT le_2
		MOV EBX,0A00h
le_2:
		MOV DWORD PTR DS:[ESI+4],EBX
		MOV EBX,83000000h
		MOV ECX,3C0h
		PXOR MM6,MM6
jump_0173D005:
		MOVQ QWORD PTR DS:[ESI+18h],MM2
		MOVD MM4,EAX
		MOVQ MM2,MM1
		PUNPCKLWD MM1,MM6
		PADDSW MM2,MM4
		MOVD MM5,EBX
		MOV EDI,8000h
		PSRLQ MM1,20h
		PSUBSW MM2,MM4
		MOVD MM0,EDI
		PADDSW MM2,MM5
		MOVD EBX,MM1
		PSUBSW MM2,MM5
		SUB ECX,EBX
		MOV EDX,7FFFh
		SUB EDX,ECX
		ADD EDI,ECX
		MOVD MM5,EDX
		MOV EAX,DWORD PTR DS:[ESI+8]
		PADDSW MM2,MM5
		MOV EBX,Local20
		PSUBSW MM2,MM5
		MOV DWORD PTR DS:[ESI+0Ch],EAX
		NOP
		MOV DWORD PTR DS:[ESI+8],EBX
		NOP
		MOVD DWORD PTR DS:[ESI+10h],MM2
		PUSHAD
		MOV EDI,Local30
		MOVSX EAX,WORD PTR DS:[EDI+10h]
		MOVSX EBX,WORD PTR DS:[EDI+12h]
		CMP EBX,300h
		JLE SHORT le_3
		MOV EBX,300h
le_3:
		CMP EBX,-300h
		JGE SHORT ge_3
		MOV EBX,-300h
ge_3:
		MOV ECX,3C0h
		SUB ECX,EBX
		CMP EAX,ECX
		JLE SHORT jump_0173D089
		MOV EAX,ECX
jump_0173D089:
		MOV EDX,EAX
		NEG EDX
		CMP EDX,ECX
		JLE SHORT jump_0173D095
		MOV EAX,ECX
		NEG EAX
jump_0173D095:
		MOV WORD PTR DS:[EDI+10h],AX
		MOV WORD PTR DS:[EDI+12h],BX
		POPAD
		MOV EAX,Local0C
		MOV EBX,Local24
		INC EBX
		INC EAX
		CMP EBX,4
		MOV Local24,EBX
		MOV Local0C,EAX
		JNZ innerloop1_start

		MOV EBX,0
		MOV Local24,EBX

		; Right samples
innerloop2_start: ; 0173CCE6
		MOV EDI,Local1C
		MOV EBX,DWORD PTR DS:[EDI+EBX*8+4]
		MOV EAX,Local28
		MOVD MM0,DWORD PTR DS:[EAX+10h]
		MOVD MM6,DWORD PTR DS:[EAX+20h]
		MOVQ MM2,QWORD PTR DS:[EAX+18h]
		MOVQ MM4,QWORD PTR DS:[EAX+28h]
		PMADDWD MM0,MM6
		MOV ESI,Local24
		SUB EBX,7
		MOV ECX,DWORD PTR DS:[EAX+4]
		MOV EDX,EBX
		MOV DWORD PTR DS:[EDI+ESI*8+4],EBX
		MOV EDI,EBX
		SAR EDX,1Fh
		MOV SI,WORD PTR DS:[EAX+2Eh]
		XOR EBX,EDX
		MOVQ MM3,QWORD PTR DS:[EAX+28h]
		PSRAD MM0,0Ah
		SUB EBX,EDX
		PMADDWD MM2,MM3
		MOV Local3C,EBX
		MOV EAX,EDI
		MOV EBX,DWORD PTR DS:[EBX*4+Lookup1774338]
		XOR EDX,EDX
		MOVD Local38,MM0
		MOVQ MM1,MM2
		ADD EBX,ECX
		PUNPCKHDQ MM1,MM2
		MOV EDX,EBX
		PADDD MM1,MM2
		AND EBX,0FFFFFF00h
		PSRAD MM1,0Ah
		SAR EAX,1Fh
		SUB EDX,EBX
		LEA EDI,Lookup1774648
		AND EAX,84h
		SAR EDX,3
		ADD EDI,EAX
		MOVD Local20,MM1
		PSLLQ MM4,10h
		MOV ECX,EBX
		DEC EBX
		SAR ECX,8
		MOV EDX,DWORD PTR DS:[EDI+EDX*4]
		SAR EBX,1Fh
		MOV EAX,Local28
		SHL EDX,CL
		SAR EDX,0Ah
		XOR EBX,0FFFFFFFFh
		MOV ECX,Local20
		AND EDX,EBX
		MOV EBX,Local38
		ADD ECX,EDX
		ADD ECX,EBX
		MOV Local34,EDX
		MOVD MM5,ECX
		PSLLQ MM6,10h
		MOVD MM0,Local34
		PSLLD MM5,10h
		MOVQ MM2,QWORD PTR DS:[EAX+8]
		PSLLD MM0,10h
		MOVQ MM3,Constant17747A0
		PSRLD MM5,10h
		PSRLD MM0,10h
		POR MM5,MM6
		POR MM4,MM0
		PADDW MM1,MM0
		MOVD DWORD PTR DS:[EAX+20h],MM5
		PACKSSDW MM2,MM5
		MOVQ QWORD PTR DS:[EAX+28h],MM4
		PSLLQ MM2,10h
		MOV WORD PTR DS:[EAX+30h],SI
		MOV ESI,Local0C
		MOV EBX,Local2C
		MOV WORD PTR DS:[EBX+ESI*2],CX ; Output
		MOVD EBX,MM1
		PSLLD MM1,10h
		AND EBX,0FFFFh
		JE jump_0173CF42b
		MOVQ MM0,QWORD PTR DS:[EAX+10h]
		PSRAD MM1,1Fh
		POR MM1,Constant17747B0
		PADDSW MM4,MM2
		PSRAW MM4,0Fh
		MOVQ MM5,MM3
		POR MM4,Constant17747B0
		PSLLD MM0,10h
		MOVQ MM7,Constant17747A8
		PSRLQ MM4,10h
		PMULLW MM4,MM1
		MOV ESI,Local28
		MOVQ MM1,QWORD PTR DS:[EAX+10h]
		PSRLQ MM5,20h
		PSLLQ MM1,20h
		MOV EAX,7CFF0000h
		MOVQ MM2,MM4
		PSRLQ MM1,30h
		PAND MM4,Constant17747C0
		POR MM4,MM0
		MOV EBX,Local3C
		MOVD MM6,DWORD PTR DS:[ESI+4]
		PMADDWD MM3,MM4
		MOV ECX,0F6h
		MOV EBX,DWORD PTR DS:[EBX*4+Lookup1774378]
		MOVD MM4,ECX
		PMADDWD MM6,MM4
		PSRLD MM3,8
		MOVQ MM4,MM3
		PSLLD MM3,2
		MOVD EDI,MM6
		PADDSW MM3,MM7
		PSUBSW MM3,MM7
		PSRLQ MM7,20h
		MOVQ MM0,QWORD PTR DS:[ESI+28h]
		PSUBSW MM3,MM7
		PSLLQ MM0,30h
		PADDSW MM3,MM7
		MOVQ MM6,MM0
		PAND MM3,Constant17747C0
		PSRLQ MM0,20h
		POR MM3,Constant17747B8
		POR MM0,MM6
		PMULLW MM3,MM2
		ADD EBX,EDI
		PSRAD MM0,20h
		SAR EBX,8
		MOVQ MM2,MM3
		PAND MM0,Constant17747D8
		PSRLQ MM2,10h
		MOVQ MM6,QWORD PTR DS:[ESI+2Ah]
		PSUBW MM2,MM3
		PCMPGTW MM6,Constant17747E8
		PSLLD MM2,10h
		PANDN MM6,Constant17747E8
		POR MM1,MM2
		PADDUSW MM6,Constant17747B0
		PMADDWD MM1,MM5
		CMP EBX,10Fh
		JGE SHORT ge_1b
		MOV EBX,10Fh
ge_1b: ; 0173CEF1
		CMP EBX,0A00h
		JLE SHORT le_1b
		MOV EBX,0A00h
le_1b: ; 0173CEFE
		MOV DWORD PTR DS:[ESI+4],EBX
		MOVQ MM2,Constant17747D0
		PSRAD MM1,8
		POR MM0,Constant17747E0
		PSLLD MM1,10h
		PMULLW MM2,QWORD PTR DS:[ESI+18h]
		PSLLD MM4,10h
		MOV ECX,3C0h
		MOV EBX,83000000h
		PMULLW MM0,MM6
		PXOR MM6,MM6
		PSRLD MM4,10h
		PADDSW MM2,MM0
		POR MM1,MM4
		PSRAW MM2,8
		JMP jump_0173D005b
jump_0173CF42b:
		MOVQ MM1,QWORD PTR DS:[EAX+10h]
		MOVD MM3,Constant17747C8
		MOVQ MM2,MM1
		PMULLW MM1,MM3
		MOV ESI,Local28
		MOV EAX,7CFF0000h
		PMULHW MM2,MM3
		PUNPCKLWD MM1,MM2
		PSRAD MM1,8
		PACKSSDW MM1,MM3
		MOVQ MM0,QWORD PTR DS:[ESI+28h]
		MOVD MM7,DWORD PTR DS:[ESI+4]
		PSLLQ MM0,30h
		MOV ECX,0F6h
		MOV EBX,Local3C
		MOVD MM4,ECX
		MOVQ MM6,MM0
		PSRLQ MM0,20h
		MOV EBX,DWORD PTR DS:[EBX*4+Lookup1774378]
		POR MM0,MM6
		MOVQ MM6,QWORD PTR DS:[ESI+2Ah]
		PSRAD MM0,20h
		PAND MM0,Constant17747D8
		PMADDWD MM7,MM4
		MOVQ MM2,Constant17747D0
		PCMPGTW MM6,Constant17747E8
		PANDN MM6,Constant17747E8
		PMULLW MM2,QWORD PTR DS:[ESI+18h]
		POR MM0,Constant17747E0
		PADDUSW MM6,Constant17747B0
		MOVD EDI,MM7
		PMULLW MM0,MM6
		ADD EBX,EDI
		SAR EBX,8
		CMP EBX,10Fh
		JGE SHORT ge_2b
		MOV EBX,10Fh
ge_2b:
		PADDSW MM2,MM0
		CMP EBX,0A00h
		PSRAW MM2,8
		JLE SHORT le_2b
		MOV EBX,0A00h
le_2b:
		MOV DWORD PTR DS:[ESI+4],EBX
		MOV EBX,83000000h
		MOV ECX,3C0h
		PXOR MM6,MM6
jump_0173D005b:
		MOVQ QWORD PTR DS:[ESI+18h],MM2
		MOVD MM4,EAX
		MOVQ MM2,MM1
		PUNPCKLWD MM1,MM6
		PADDSW MM2,MM4
		MOVD MM5,EBX
		MOV EDI,8000h
		PSRLQ MM1,20h
		PSUBSW MM2,MM4
		MOVD MM0,EDI
		PADDSW MM2,MM5
		MOVD EBX,MM1
		PSUBSW MM2,MM5
		SUB ECX,EBX
		MOV EDX,7FFFh
		SUB EDX,ECX
		ADD EDI,ECX
		MOVD MM5,EDX
		MOV EAX,DWORD PTR DS:[ESI+8]
		PADDSW MM2,MM5
		MOV EBX,Local20
		PSUBSW MM2,MM5
		MOV DWORD PTR DS:[ESI+0Ch],EAX
		NOP
		MOV DWORD PTR DS:[ESI+8],EBX
		NOP
		MOVD DWORD PTR DS:[ESI+10h],MM2
		PUSHAD
		MOV EDI,Local28
		MOVSX EAX,WORD PTR DS:[EDI+10h]
		MOVSX EBX,WORD PTR DS:[EDI+12h]
		CMP EBX,300h
		JLE SHORT le_3b
		MOV EBX,300h
le_3b:
		CMP EBX,-300h
		JGE SHORT ge_3b
		MOV EBX,-300h	
ge_3b:
		MOV ECX,3C0h
		SUB ECX,EBX
		CMP EAX,ECX
		JLE SHORT jump_0173D462b
		MOV EAX,ECX
jump_0173D462b:
		MOV EDX,EAX
		NEG EDX
		CMP EDX,ECX
		JLE SHORT jump_0173D46Eb
		MOV EAX,ECX
		NEG EAX
jump_0173D46Eb:
		MOV WORD PTR DS:[EDI+10h],AX
		MOV WORD PTR DS:[EDI+12h],BX
		POPAD
		MOV EAX,Local0C
		MOV EBX,Local24
		INC EBX
		INC EAX
		MOV Local24,EBX
		CMP EBX,4
		MOV Local0C,EAX
		JNZ innerloop2_start

		MOV EDI,Local1C
		ADD EDI,20h
		MOV Local1C,EDI
		MOV ECX,Local10
		CMP EAX,ECX
		JNZ upperloop_start
		EMMS

		MOV ECX,0
		MOV EBX,SampleCount
		MOV ESI,Output
		MOV EDX,Output
smallloop_start:
		MOVQ MM0,QWORD PTR DS:[ESI]
		MOVQ MM1,QWORD PTR DS:[ESI+8]
		MOVQ MM2,MM0
		PADDSW MM0,MM1
		ADD ESI,10h
		PSUBSW MM2,MM1
		MOVQ MM3,MM0
		PUNPCKLWD MM3,MM2
		ADD EDX,10h
		PUNPCKHWD MM0,MM2
		ADD ECX,8
		MOVQ QWORD PTR DS:[EDX-10h],MM3
		MOVQ QWORD PTR DS:[EDX-8h],MM0
		CMP ECX,EBX
		JL SHORT smallloop_start
		EMMS
	};
	return;
}

static long DoMono4BitSample(long Input, S4BitAdpcmBlockHeader& Header)
{
	//S4BitAdpcmBlockHeader* Local30=&Left;
	//S4BitAdpcmBlockHeader* Local28=&Right;
	//long* Local1C=(long*)Expanded;
	long Local24=0;
	long Local20=0;
	long Local0C=0;
	long Local3C=0;
	long Local38=0;
	long Local34=0;
	unsigned long Local08=0;
	//unsigned long Local10=SampleCount;

	const unsigned long long Constant17747A0=0x000200FE00FF0C00;
	const unsigned long long Constant17747A8=0x7800780077FF77FF;
	const unsigned long long Constant17747B0=0x0001000100010001;
	const unsigned long long Constant17747B8=0x0000000004000000;
	const unsigned long long Constant17747C0=0x000000000000FFFF;
	const unsigned long long Constant17747D0=0x00FF00FF00FF00FF;
	const unsigned long long Constant17747D8=0xF000F000F000F000;
	const unsigned long long Constant17747E0=0x0800080008000800;
	const unsigned long long Constant17747E8=0xFFFFFFFFFFFFFFFF;
	const unsigned long Constant17747C8=0x00FE00FF;

	__asm
	{
		MOV EAX,Header
		MOV EBX,Input
		MOVD MM0,DWORD PTR DS:[EAX+10h]
		MOVD MM6,DWORD PTR DS:[EAX+20h]
		MOVQ MM2,QWORD PTR DS:[EAX+18h]
		MOVQ MM4,QWORD PTR DS:[EAX+28h]
		PMADDWD MM0,MM6
		SUB EBX,7
		MOV ECX,DWORD PTR DS:[EAX+4]
		MOV EDX,EBX
		MOV Input,EBX
		SAR EDX,1Fh
		MOV SI,WORD PTR DS:[EAX+2Eh]
		XOR EBX,EDX
		MOVQ MM3,QWORD PTR DS:[EAX+28h]
		PSRAD MM0,0Ah
		SUB EBX,EDX
		PMADDWD MM2,MM3
		MOV Local3C,EBX
		MOV EAX,Input
		MOV EBX,DWORD PTR DS:[EBX*4+Lookup1774338]
		XOR EDX,EDX
		MOVD Local38,MM0
		MOVQ MM1,MM2
		ADD EBX,ECX
		PUNPCKHDQ MM1,MM2
		MOV EDX,EBX
		PADDD MM1,MM2
		AND EBX,0FFFFFF00h
		PSRAD MM1,0Ah
		SAR EAX,1Fh
		SUB EDX,EBX
		LEA EDI,Lookup1774648
		AND EAX,84h
		SAR EDX,3
		ADD EDI,EAX
		MOVD Local20,MM1
		PSLLQ MM4,10h
		MOV ECX,EBX
		DEC EBX
		SAR ECX,8
		MOV EDX,DWORD PTR DS:[EDI+EDX*4]
		SAR EBX,1Fh
		MOV EAX,Header
		SHL EDX,CL
		SAR EDX,0Ah
		XOR EBX,0FFFFFFFFh
		MOV ECX,Local20
		AND EDX,EBX
		MOV EBX,Local38
		ADD ECX,EDX
		ADD ECX,EBX
		MOV Local34,EDX
		MOVD MM5,ECX
		PSLLQ MM6,10h
		MOVD MM0,Local34
		PSLLD MM5,10h
		MOVQ MM2,QWORD PTR DS:[EAX+8]
		PSLLD MM0,10h
		MOVQ MM3,Constant17747A0
		PSRLD MM5,10h
		PSRLD MM0,10h
		POR MM5,MM6
		POR MM4,MM0
		PADDW MM1,MM0
		MOVD DWORD PTR DS:[EAX+20h],MM5
		PACKSSDW MM2,MM5
		MOVQ QWORD PTR DS:[EAX+28h],MM4
		PSLLQ MM2,10
		MOV WORD PTR DS:[EAX+30h],SI
		MOV Local08,ECX
		MOVD EBX,MM1
		PSLLD MM1,10h
		AND EBX,0FFFFh
		JE jump_0173CF42
		MOVQ MM0,QWORD PTR DS:[EAX+10h]
		PSRAD MM1,1Fh
		POR MM1,Constant17747B0
		PADDSW MM4,MM2
		PSRAW MM4,0Fh
		MOVQ MM5,MM3
		POR MM4,Constant17747B0
		PSLLD MM0,10h
		MOVQ MM7,Constant17747A8
		PSRLQ MM4,10h
		PMULLW MM4,MM1
		MOV ESI,Header
		MOVQ MM1,QWORD PTR DS:[EAX+10h]
		PSRLQ MM5,20h
		PSLLQ MM1,20h
		MOV EAX,7CFF0000h
		MOVQ MM2,MM4
		PSRLQ MM1,30h
		PAND MM4,Constant17747C0
		POR MM4,MM0
		MOV EBX,Local3C
		MOVD MM6,DWORD PTR DS:[ESI+4]
		PMADDWD MM3,MM4
		MOV ECX,0F6h
		MOV EBX,DWORD PTR DS:[EBX*4+Lookup1774378]
		MOVD MM4,ECX
		PMADDWD MM6,MM4
		PSRLD MM3,8
		MOVQ MM4,MM3
		PSLLD MM3,2
		MOVD EDI,MM6
		PADDSW MM3,MM7
		PSUBSW MM3,MM7
		PSRLQ MM7,20h
		MOVQ MM0,QWORD PTR DS:[ESI+28h]
		PSUBSW MM3,MM7
		PSLLQ MM0,30h
		PADDSW MM3,MM7
		MOVQ MM6,MM0
		PAND MM3,Constant17747C0
		PSRLQ MM0,20h
		POR MM3,Constant17747B8
		POR MM0,MM6
		PMULLW MM3,MM2
		ADD EBX,EDI
		PSRAD MM0,20h
		SAR EBX,8
		MOVQ MM2,MM3
		PAND MM0,Constant17747D8
		PSRLQ MM2,10h
		MOVQ MM6,QWORD PTR DS:[ESI+2Ah]
		PSUBW MM2,MM3
		PCMPGTW MM6,Constant17747E8
		PSLLD MM2,10h
		PANDN MM6,Constant17747E8
		POR MM1,MM2
		PADDUSW MM6,Constant17747B0
		PMADDWD MM1,MM5
		CMP EBX,10Fh
		JGE SHORT ge_1
		MOV EBX,10Fh
ge_1: ; 0173CEF1
		CMP EBX,0A00h
		JLE SHORT le_1
		MOV EBX,0A00h
le_1: ; 0173CEFE
		MOV DWORD PTR DS:[ESI+4],EBX
		MOVQ MM2,Constant17747D0
		PSRAD MM1,8
		POR MM0,Constant17747E0
		PSLLD MM1,10h
		PMULLW MM2,QWORD PTR DS:[ESI+18h]
		PSLLD MM4,10h
		MOV ECX,3C0h
		MOV EBX,83000000h
		PMULLW MM0,MM6
		PXOR MM6,MM6
		PSRLD MM4,10h
		PADDSW MM2,MM0
		POR MM1,MM4
		PSRAW MM2,8
		JMP jump_0173D005
jump_0173CF42:
		MOVQ MM1,QWORD PTR DS:[EAX+10h]
		MOVD MM3,Constant17747C8
		MOVQ MM2,MM1
		PMULLW MM1,MM3
		MOV ESI,Header
		MOV EAX,7CFF0000h
		PMULHW MM2,MM3
		PUNPCKLWD MM1,MM2
		PSRAD MM1,8
		PACKSSDW MM1,MM3
		MOVQ MM0,QWORD PTR DS:[ESI+28h]
		MOVD MM7,DWORD PTR DS:[ESI+4]
		PSLLQ MM0,30h
		MOV ECX,0F6h
		MOV EBX,Local3C
		MOVD MM4,ECX
		MOVQ MM6,MM0
		PSRLQ MM0,20h
		MOV EBX,DWORD PTR DS:[EBX*4+Lookup1774378]
		POR MM0,MM6
		MOVQ MM6,QWORD PTR DS:[ESI+2Ah]
		PSRAD MM0,20h
		PAND MM0,Constant17747D8
		PMADDWD MM7,MM4
		MOVQ MM2,Constant17747D0
		PCMPGTW MM6,Constant17747E8
		PANDN MM6,Constant17747E8
		PMULLW MM2,QWORD PTR DS:[ESI+18h]
		POR MM0,Constant17747E0
		PADDUSW MM6,Constant17747B0
		MOVD EDI,MM7
		PMULLW MM0,MM6
		ADD EBX,EDI
		SAR EBX,8
		CMP EBX,10Fh
		JGE SHORT ge_2
		MOV EBX,10Fh
ge_2:
		PADDSW MM2,MM0
		CMP EBX,0A00h
		PSRAW MM2,8
		JLE SHORT le_2
		MOV EBX,0A00h
le_2:
		MOV DWORD PTR DS:[ESI+4],EBX
		MOV EBX,83000000h
		MOV ECX,3C0h
		PXOR MM6,MM6
jump_0173D005:
		MOVQ QWORD PTR DS:[ESI+18h],MM2
		MOVD MM4,EAX
		MOVQ MM2,MM1
		PUNPCKLWD MM1,MM6
		PADDSW MM2,MM4
		MOVD MM5,EBX
		MOV EDI,8000h
		PSRLQ MM1,20h
		PSUBSW MM2,MM4
		MOVD MM0,EDI
		PADDSW MM2,MM5
		MOVD EBX,MM1
		PSUBSW MM2,MM5
		SUB ECX,EBX
		MOV EDX,7FFFh
		SUB EDX,ECX
		ADD EDI,ECX
		MOVD MM5,EDX
		MOV EAX,DWORD PTR DS:[ESI+8]
		PADDSW MM2,MM5
		MOV EBX,Local20
		PSUBSW MM2,MM5
		MOV DWORD PTR DS:[ESI+0Ch],EAX
		NOP
		MOV DWORD PTR DS:[ESI+8],EBX
		NOP
		MOVD DWORD PTR DS:[ESI+10h],MM2
		EMMS
		MOV ECX,Header
		MOV SI,WORD PTR DS:[ECX+12h]
		MOVSX EAX,SI
		CDQ
		XOR EAX,EDX
		SUB EAX,EDX
		CMP EAX,300h
		JLE jump_010AC670
		XOR EAX,EAX
		TEST SI,SI
		SETGE AL
		LEA EAX,DWORD PTR DS:[EAX+EAX-1]
		LEA EAX,DWORD PTR DS:[EAX+EAX*2]
		SHL EAX,8
		MOV WORD PTR DS:[ECX+12h],AX
jump_010AC670:
		MOV SI,WORD PTR DS:[ECX+10h]
		MOV DI,WORD PTR DS:[ECX+12h]
		MOVSX EAX,SI
		CDQ
		XOR EAX,EDX
		MOV EBX,3C0h
		SUB EAX,EDX
		MOVSX EDX,DI
		SUB EBX,EDX
		CMP EAX,EBX
		JLE jump_010AC6A8
		XOR EAX,EAX
		MOV EDX,3C0h
		TEST SI,SI
		SETGE AL
		SUB EDX,EDI
		LEA EAX,DWORD PTR DS:[EAX+EAX-1]
		IMUL EAX,EDX
		MOV WORD PTR DS:[ECX+10h],AX
jump_010AC6A8:
	};
	return Local08;
}

void DecompressMono4BitAdpcmBlock(S4BitAdpcmBlockHeader& Header, \
								  unsigned long* Expanded, short* Output, \
								  unsigned long SampleCount)
{
	// Sorry for this crappy mess
	// I should actually convert it to C++, but it works so what the heck

	short* Local2C=Output;
	S4BitAdpcmBlockHeader* Local30=&Header;
	long* Local1C=(long*)Expanded;
	unsigned long Local10=SampleCount;
	long Local18;
	long Local0C;

	__asm
	{
		XOR EBX,EBX
		MOV EDI,Local2C

loop_top:
		MOV ECX,Local1C
		MOV EAX,Local30
		PUSH EAX
		MOV EDX,DWORD PTR DS:[ECX+EBX*4h]
		PUSH EDX
		CALL DoMono4BitSample
		AND EAX,0FFFFh
		MOV DWORD PTR DS:[EDI],EAX
		MOV ECX,Local1C
		MOV EAX,Local30
		MOV EDX,DWORD PTR DS:[ECX+EBX*4h+4h]
		PUSH EAX
		PUSH EDX
		CALL DoMono4BitSample
		MOV ECX,DWORD PTR DS:[EDI]
		ADD EBX,2h
		SHL EAX,10h
		OR ECX,EAX
		MOV EAX,Local10
		MOV DWORD PTR DS:[EDI],ECX
		ADD ESP,10h
		ADD EDI,4h
		CMP EBX,EAX
		JB loop_top
	};
	/*__asm
	{
		XOR ECX,ECX ;
		XOR EDX,EDX ;
		XOR EBX,EBX ;
		MOV EDI,Local2C

loop_top:
		CMP EDX,EBX
		MOV Local18,EBX
		JLE other_jump
		MOV Local0C,EBX
		LEA EBX,DWORD PTR DS:[ECX*4+4]
inner_loop:
		MOV EAX,Header
		MOV EDX,Local0C
		MOV ECX,Input
		ADD EAX,EDX
		PUSH EAX
		MOV EDX,DWORD PTR DS:[EBX+ECX-4]
		PUSH EDX
		;CALL DWORD PTR SS:[EBP-8]
		MOV ECX,Local0C
		AND EAX,0FFFFh
		MOV DWORD PTR DS:[EDI],EAX
		MOV EAX,Header
		LEA EDX,DWORD PTR DS:[ECX+EAX+34]
		MOV EAX,Input
		PUSH EDX
		MOV ECX,DWORD PTR DS:[EBX+EAX]
		PUSH ECX
		;CALL DWORD PTR SS:[EBP-8]
		MOV ECX,DWORD PTR DS:[EDI]
		MOV EDX,Local0C
		SHL EAX,10h
		OR ECX,EAX
		MOV EAX,Local18
		ADD EDX,68h
		MOV DWORD PTR DS:[EDI],ECX
		MOV Local0C,EDX
		;MOV EDX,DWORD PTR DS:[ESI+50]
		ADD EAX,2
		ADD ESP,10
		;MOV ECX,DWORD PTR DS:[EDX+Ch]
		ADD EDI,4
		ADD EBX,8
		CMP EAX,ECX
0173CA74  |. 8945 E8        ||MOV DWORD PTR SS:[EBP-18],EAX
0173CA77  |.^7C A2          |\JL SHORT SNDDSoun.0173CA1B
0173CA79  |. 8B4D EC        |MOV ECX,DWORD PTR SS:[EBP-14]
0173CA7C  |. 8B45 FC        |MOV EAX,DWORD PTR SS:[EBP-4]
0173CA7F  |. 33DB           |XOR EBX,EBX
other_jump:
0173CA81  |> 8B56 50        |MOV EDX,DWORD PTR DS:[ESI+50]
0173CA84  |. 8B52 0C        |MOV EDX,DWORD PTR DS:[EDX+C]
0173CA87  |. 03CA           |ADD ECX,EDX
0173CA89  |. 3BC8           |CMP ECX,EAX
0173CA8B  |. 894D EC        |MOV DWORD PTR SS:[EBP-14],ECX
0173CA8E  |.^0F82 76FFFFFF  \JB loop_top
	};*/
	return;
}
