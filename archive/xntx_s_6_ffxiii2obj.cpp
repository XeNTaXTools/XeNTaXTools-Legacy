// ffxiii2obj.cpp
//

#ifdef _MSC_VER
#undef UNICODE
#undef _UNICODE
#endif

#include <io.h>
#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;

typedef struct 
{
   uint16 stream;
   uint16 offset;
   uint32 type;
   uint32 count;
   uint16 usage;
   uint16 index;
} t_chunk;

typedef struct{
	char magic[4];	// STMS
	int  zeros;		// zeros
	int  size1;		// chunk size
	int  size2;		// chunk size again
	int  dcount;	// chunk count
	int  count;		// element count
	int  size;		// element size
} t_header;

typedef struct {
	unsigned short v1,v2,v3;
} t_face;

typedef struct {
	short x,y,z;
} t_vertex;

unsigned int rev32u (unsigned int i)
{
	unsigned char b1, b2, b3, b4;
	b1 = i & 255;
	b2 = ( i >> 8 ) & 255;
	b3 = ( i>>16 ) & 255;
	b4 = ( i>>24 ) & 255;
	return ((unsigned int)b1 << 24) + ((unsigned int)b2 << 16) + ((unsigned int)b3 << 8) + b4;

}

unsigned short rev16u (unsigned short i)
{
	unsigned char b1, b2;
	b1 = i & 255;
	b2 = ( i >> 8 ) & 255;
	return ((unsigned short)b1 << 8) + b2;

}

short rev16s (unsigned short i)
{
	unsigned char b1, b2;
	b1 = i & 255;
	b2 = ( i >> 8 ) & 255;
	return ((short)b1 << 8) + b2;

}

union U {
    int i;
    float f;
};


float h2f(unsigned short y)
{

	y=rev16u(y);

	U value;

    int s = (y >> 15) & 0x00000001;
    int e = (y >> 10) & 0x0000001f;
    int m =  y        & 0x000003ff;

    if (e == 0)
    {
	if (m == 0)
	{
	    //
	    // Plus or minus zero
	    //

	    return s << 31;
	}
	else
	{
	    //
	    // Denormalized number -- renormalize it
	    //

	    while (!(m & 0x00000400))
	    {
		m <<= 1;
		e -=  1;
	    }

	    e += 1;
	    m &= ~0x00000400;
	}
    }
    else if (e == 31)
    {
	if (m == 0)
	{
	    //
	    // Positive or negative infinity
	    //

	    return (s << 31) | 0x7f800000;
	}
	else
	{
	    //
	    // Nan -- preserve sign and significand bits
	    //

	    return (s << 31) | 0x7f800000 | (m << 13);
	}
    }

    //
    // Normalized number
    //

    e = e + (127 - 15);
    m = m << 13;

    //
    // Assemble s, e and m.
    //

	value.i = (s << 31) | (e << 23) | m;
    return value.f;

}

long find_string (unsigned char *buf, size_t start,
						 size_t len, const char *s)
{
	long i, j;
	int slen = strlen(s);
	long imax = len - slen - 1;
	long ret = -1;
	int match;

	for (i=start; i<imax; i++) {
		match = 1;
		for (j=0; j<slen; j++) {
			if (buf[i+j] != s[j]) {
				match = 0;
				break;
			}
		}
		if (match) {
			ret = i;
			break;
		}
	}

	return ret;
}


int _tmain(int argc, _TCHAR* argv[])
{
	FILE * in;
	FILE * out;

	char stms[] = "STMS\0";

	int  filelen;
	int  offs=0;
	int  voffs=0;
	int  noffs=0;
	int  moffs=0;
	int  block=0;
	int  i,j;
	int  count;
	int  faces=1;
	int  vhsize;//vertex header size

	t_header * f;
	t_header * v;
	t_chunk  * c;

	t_face *face;
	t_vertex *vert;


	unsigned char *buf=0;

	if (argc!=2)
	{
		printf("invalid parameters: use '%s filename'\n",argv[0]);
		return 1;
	}

	in = fopen(argv[1],"rb");

//	in = fopen("release/c001.x360.trb","rb");
	if (!in) {
		printf ("invalid input file\n");
		return 1;
	}

	filelen = filelength (fileno (in));

	if (!filelen) {
		printf ("invalid input file\n");
		return 1;
	}

	buf = (unsigned char*)malloc (filelen);
	if (!buf)
	{
		printf ("not enough memory\n");
		return 1;
	}

	fread(buf,1,filelen,in);
	fclose (in);

	offs = find_string(buf, offs, filelen, stms) ;
	while (offs != -1) {

		if (!(block%2)){	// face list (start new mesh)
			f = (t_header*)(buf+offs);
			printf("g mesh%d\n",(block+1)/2+1);

		} else {			// vertices list
			v = (t_header*)(buf+offs);

			v->size = rev32u(v->size);
			vhsize = (v->size + 4) * 4;

			count = rev32u(v->count);
			v->dcount = rev32u(v->dcount);

			for (j=0; j<v->dcount; j++) {
				c = (t_chunk*)(buf+offs+32+j*16);
				switch (c->usage) {
					case 0x0000:
						voffs = rev16u(c->offset);
						break;
					case 0x0200:
						noffs = rev16u(c->offset);
						break;
					case 0x0800:
						moffs = rev16u(c->offset);
						break;
					default:
						break;
				}

			}
			
			offs += vhsize;


			for (i=0;i<count;i++){
				
					vert = (t_vertex*)(buf+offs+voffs+i*v->size);
					printf("v %d %d %d\n",rev16s(vert->x), rev16s(vert->y), rev16s(vert->z));

					vert = (t_vertex*)(buf+offs+moffs+i*v->size);
					printf("vt %.6f %.6f\n",h2f(vert->x), h2f(vert->y));

//					vert = (t_vertex*)(buf+offs+noffs+i*v->size);
//					printf("vn %d %d %d\n",rev16u(vert->x), rev16u(vert->y), rev16u(vert->z));
			}



			count = rev32u(f->count)/3;
			for (i=0;i<count;i++){
				face = (t_face*)((unsigned char*)f+48+i*6);
				printf ("f %d/%d/%d %d/%d/%d %d/%d/%d\n",rev16u(face->v1)+faces,rev16u(face->v1)+faces,rev16u(face->v1)+faces,
														rev16u(face->v2)+faces, rev16u(face->v2)+faces, rev16u(face->v2)+faces, 
														rev16u(face->v3)+faces, rev16u(face->v3)+faces, rev16u(face->v3)+faces);
			}
			faces+=rev32u(v->count);

		}

		offs = find_string(buf, offs+1, filelen, stms) ;
		block++; //next block
	}


	return 0;
}

