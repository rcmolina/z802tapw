/*
    This file is part of the registered Spectrum emulator package 'Z80'
    version 3.02, and may not be distributed.  You may use this source for
    other PC based Spectrum or Z80 emulators only after permission.  It is
    however permitted to use this source file or parts thereof for Spectrum
    or Z80 emulators on non-PC based machines, provided that the source is
    acknowledged.

                                                      Gerton Lunter, 29/10/94
*/

/* z802tap - converts .z80 snapshot to tape blocks including loader */

//soluciona el problema de la lectura de short ints de fichero z80, el puntero de lectura se desalinea empezando en byte par
#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

#define O_BINARY 0

#include <stdio.h>
#include <stdlib.h>
//#include <mem.h>
#include <string.h>
//#include <alloc.h>
#include <fcntl.h>
#include <sys/stat.h>
//#include <io.h>
#include <unistd.h>

struct z80header {
    char areg,freg;
    short int bcreg,hlreg,pcreg,spreg;
    char ireg,rreg;
    unsigned char flagbyte;
    short int dereg,bcareg,deareg,hlareg;
    char aareg,fareg;
    short int iyreg,ixreg;
    char iff1,iff2,imode;

    short int length,pcreg2;
    char hmode,outstate,if1paged,flagbyte2,bytefffd;
    char soundreg[16];

    short int tstates;
    char viertel;
    char zero;
    char discflg;
    char m128flg;
    char rompage1,rompage2;
    short int udef_joy[5];
    short int udef_keys[5];
    char disctype;
    char discinhibit;
    char discinhibitflg;
};

struct blockheader {
       unsigned short int length;
       char page;
};


struct tapeh {
       char type;
       char fnam[10];
       short int length;
       char b0,b1,b2,b3;
};

const unsigned char loadline1[]={0xd2,0,0,1,0xfd,0x7f,0x3e,0x8f,0x2e,0,0x3b,0xed,0x69,0x80,0x06,0xbf,
      0xed,0x79,6,0xff,0xe1,0x38,0xf3,0x3e,0,0xd3,0xfe,0xc1,0xd1,0xc3,0xe0,0x56};
const unsigned char loadline2[]={0xd9,1,0,0,0x11,0,0,0x21,0,0,0xdd,0x21,0,0,0xfd,0xe1,
      0x3e,0,0xed,0x4f,0xf1,8,0xf1,0x31,0,0,0xed,0x56,0xfb,0xc3,0,0};
const unsigned char LOADER[]={
0x13,0x00,0x00,0x00,0x4C,0x6F,0x61,0x64,0x65,0x72,0x20,0x20,0x20,0x20,0x90,0x01,0x0A,0x00,0x90,0x01,0x3B,0x92,0x01,0xFF,0x00,0x0A,0x8C,0x01,0xE7,0x31,0x0E,0x00, 
0x00,0x01,0x00,0x00,0x3A,0xDA,0x31,0x0E,0x00,0x00,0x01,0x00,0x00,0x3A,0xFD,0x32,0x35,0x30,0x30,0x30,0x0E,0x00,0x00,0xA8,0x61,0x00,0x3A,0xF5,0xAC,0x31,0x0E,0x00, 
0x00,0x01,0x00,0x00,0x2C,0x31,0x0E,0x00,0x00,0x01,0x00,0x00,0x3B,0xD9,0x36,0x0E,0x00,0x00,0x06,0x00,0x00,0x3B,0x22,0x4C,0x6F,0x61,0x64,0x69,0x6E,0x67,0x20,0x2E, 
0x5A,0x38,0x30,0x20,0x65,0x6D,0x75,0x6C,0x61,0x74,0x6F,0x72,0x20,0x73,0x6E,0x61,0x70,0x73,0x68,0x6F,0x74,0x22,0x3A,0xF9,0xC0,0x32,0x33,0x38,0x36,0x37,0x0E,0x00, 
0x00,0x3B,0x5D,0x00,0x3A,0xEA,0x0D,0xFF,0x21,0x3B,0x5D,0xE5,0xDD,0x21,0x00,0x7F,0x11,0x11,0x00,0xAF,0x37,0xCD,0x56,0x05,0xD0,0x3E,0x47,0xDD,0xBE,0xFE,0xC0,0x3E, 
0x4C,0xDD,0xBE,0xFF,0xC0,0xDD,0x7E,0xFC,0xFE,0x08,0x38,0x27,0x28,0x16,0xFE,0x0A,0x38,0x28,0x28,0x36,0xDD,0x21,0x00,0x40,0x11,0x00,0x1B,0x3E,0xFF,0x37,0xCD,0x56, 
0x05,0xD8,0xCF,0x1A,0xCD,0x8D,0x5D,0x21,0x00,0xC0,0x11,0x00,0x80,0x01,0x00,0x40,0xED,0xB0,0xC9,0x01,0xFD,0x7F,0xF6,0x10,0xED,0x79,0xDD,0x21,0x00,0xC0,0xED,0x5B, 
0x0B,0x7F,0xCD,0x6E,0x5D,0xAF,0xD3,0xFE,0x18,0x1B,0xDD,0x7E,0xFD,0xED,0x47,0x37,0x9F,0xDD,0x21,0x00,0x40,0x11,0x00,0x40,0x31,0xE6,0x57,0x08,0xF3,0x3E,0x0F,0xD3, 
0xFE,0xCD,0x62,0x05,0xC7,0xED,0x4B,0x0B,0x7F,0x05,0xC5,0x21,0x00,0xC0,0x09,0x11,0x00,0x7F,0x01,0x00,0x01,0xED,0xB0,0xC1,0x78,0xFE,0x3F,0x30,0x7A,0x21,0xFF,0xBF, 
0x09,0x11,0xFF,0xFF,0xED,0xB8,0x21,0x01,0x00,0x19,0x11,0x00,0xC0,0x37,0x4E,0x23,0xCB,0x11,0x18,0x08,0x14,0x28,0x60,0x15,0xCB,0x21,0x28,0xF2,0x30,0x05,0xED,0xA0, 
0x03,0x18,0xF1,0xCB,0x21,0x20,0x05,0x4E,0x23,0x37,0xCB,0x11,0x30,0x26,0xD5,0xD5,0x06,0x08,0xCD,0x37,0x5E,0xE3,0xED,0x52,0xE3,0x06,0x04,0xCD,0x37,0x5E,0xED,0x43, 
0x58,0x5E,0x22,0x56,0x5E,0x42,0x4B,0xE1,0xD1,0xED,0xB0,0x2A,0x56,0x5E,0xED,0x4B,0x58,0x5E,0x18,0xC0,0xD5,0xD5,0x06,0x0E,0xCD,0x37,0x5E,0xE3,0xA7,0xED,0x52,0xE3, 
0x06,0x0A,0x18,0xD7,0x11,0x00,0x00,0xEB,0xCB,0x21,0x20,0x05,0x1A,0x13,0x4F,0xCB,0x11,0xED,0x6A,0x10,0xF3,0xEB,0xC9,0x21,0x00,0x7F,0x11,0x00,0xFF,0x01,0x00,0x01, 
0xED,0xB0,0xC9,0x00,0x00,0x00,0x00,0x0D,0x12};


/*

loadline1:  (line 189)

#55E0   JP   NC,#0000   ;Reset on load error
        LD   BC,#7FFD   ;First OUT sets ram page
        LD   A,#8F      ;#8F+#7F = 0E, counter for 14 sound chip registers
        LD   L,#10      ;This value is sent to 7FFD (example)
#55EA   DEC  SP         ;pre-compensate for the POP below; just POP a byte
        OUT  (C),L      ;First time set ram page, then set 14 sound registers
        ADD  A,B        ;First time change 8F into 0E, then decrease A
        LD   B,#BF      ;Select sound register port
        OUT  (C),A      ;0E,...,0,FF (but then carry=0 and value doesn't get
        LD   B,#FF      ; written).  Make address for sound register value port
        POP  HL         ;Pop a word; L is sound register value.  At the end
        JR   C,#55EA    ; this pops the actual HL' value.  Cy=1 here means more
        LD   A,#00      ; sound reg values to pop. (0=example)
        OUT  (#FE),A    ;Set border colour
        POP  BC         ;Pop registers
        POP  DE
        JP   #56E0      ;Continue in line 190 (one but lowest)

loadline2:  (line 190)

#56E0   EXX             ;Those were the exchange registers
        LD   BC,#5F49   ;Just load the values of the normal registers (example)
        LD   DE,#073C   ;(example)
        LD   HL,#85FC   ;(example)
        LD   IX,#8474   ;(example)
        POP  IY         ;Pop IY.
        LD   A,#19      ;(example)
        LD   R,A        ;Set R too
        POP  AF         ;Set AF'
        EX   AF,AF'
        POP  AF         ;Set AF
        LD   SP,#5BFE   ;(example)
        IM   1          ;(example)
        DI              ;(example)
        JP   #96AC      ;Start address (example)

Stack:  space of two words for loader (used when calling ld_edge2)
        return address #55e0
        14 sound register values (bytes)
        HL',BC',DE',IY,AF',AF make 32 bytes

*/


char filename[128];
int fin,fout;
struct z80header header;
struct blockheader block;
struct tapeh tapehdr;
unsigned char *specmem[8],*temp;

//extern const char LOADER;

int main(int,char**);
int unpack(unsigned char*,unsigned char*,unsigned short int);
int pack(unsigned char*);
void subext(char*,char*);
void writetap(unsigned char*,unsigned char,unsigned short int);



void subext(char *filnam,char *ext)
//char *filnam,*ext;
{
    int i,j;
    for (i=j=0;filnam[i];i++) {
        filename[i]=filnam[i];
        if (filnam[i]=='.') j=i; else if (filnam[i]=='\\') j=0;
    }
    if (!j) j=i;
    if (j) for (i=0;ext[i];i++) filename[j++]=ext[i];
    filename[j]=0;
}


int unpack(unsigned char *inp,unsigned char *outp,unsigned short int size)
//unsigned char *inp,*outp;
//unsigned short int size;
{
    short int incount=0,outcount=0;
    short int i;
    char j;
    do {
       if ((inp[0]==0xED)&&(inp[1]==0xED)) {
            i=inp[2];
            j=inp[3];
            inp+=4;
            incount+=(short int)4;
            outcount+=i;
            for (;i!=0;i--) *(outp++)=j;
       } else {
         *(outp++)=*(inp++);
         incount++;
         outcount++;
       }
    } while (outcount<size);
    if (outcount!=size) incount=0;
    return (incount);
}


int pack(unsigned char *inp)
//unsigned char *inp;
{
    unsigned short *hash,i,j,j0,j00,thishv,input,output,output0,bits;
    unsigned short bestpos,bestlen;
    register unsigned short tmp;
    #define hv(k) (tmp=*(unsigned short *)(inp+k),16384+(((tmp>>6)^tmp)&4095))
    #define putbit(b) {if ((bits+=(bits+(b)))&0x100) \
            {temp[output0]=bits;output0=output++;bits=1;}}
    #define putbits(x,n) {unsigned short p,q=((x)<<(15-n)); \
            for (p=n;p;p--) putbit((q<<=1)>>15)}
    #define marker 0xFFFF

    hash=(unsigned short *)(temp+16384);
    for (i=0;i<16384+4096;i++) hash[i]=marker;
    input=0;
    output=1;
    output0=0;
    bits=1;

    while ((input<16384-256)&&(output<16384-256)) {
          j=j0=hash[j00=thishv=hv(input)];      /* search the hash table */
          bestlen=0;
          while ((j0!=marker)&&(bestlen<1023)) {
                j=j0;
                do {
                    if ((inp[input]==inp[j])&&
                        (inp[input+1]==inp[j+1])) {
                       for (i=2;(input+i<16384-256)&&(i<1023)&&(inp[input+i]==inp[j+i]);i++) {}
                       if (i>=bestlen) {
                          bestlen=i;
                          bestpos=j;
                       }
                       j+=i;
                    } else
                      j++;
                } while (((hv(j)==thishv)||(hv(j+1)==thishv))&&(j<input)&&(bestlen<1023));
                j0=hash[j00=j0];
          }

          if ((j==marker)||(input-j>=2)) hash[j00]=input;     /* update hash table */

          if ((bestlen>2)||((bestlen==2)&&(input-bestpos<256))) {
             putbit(0);
             if ((bestlen<16)&&(input-bestpos<256)) {   /* store result */
                putbit(1);
                putbits(input-bestpos,8);
                putbits(bestlen,4);
             } else {
               putbit(0);
               putbits(input-bestpos,14);
               putbits(bestlen,10);
             }
             input+=bestlen;
          } else {
            temp[output++]=inp[input++];
            putbit(1);
          }
    }

    do {bits<<=1;} while (!(bits&0x100));
    temp[output0]=bits;
    if (output<16384+256) {
       memcpy(temp+output,inp+input,256);
       return(output+256);
    } else return(16384);
}



void writetap(unsigned char *inp,unsigned char areg,unsigned short int size)
//unsigned char *inp;
//unsigned char areg;
//unsigned short int size;
{
    unsigned short int chksm=areg,i;
    for (i=0;i<size;i++) chksm^=inp[i];
    i=size+2;
    if (write(fout,&i,2)!=2) goto errwri;
    if (write(fout,&areg,1)!=1) {
       errwri:
       puts ("\n\nError writing to outputfile");
       exit (1);
    }
    if (write(fout,inp,size)!=size) goto errwri;
    if (write(fout,&chksm,1)!=1) goto errwri;
}




int main(int argc, char **argv)
{
    short int i,j,compr=1,scr=0;
    puts ("Z802TAP - Converts .Z80 file to .TAP file - (c) 1994 G.A. Lunter - v3.0");
    puts ("          This program may not be distributed.");
    puts ("          It is part of the registered Spectrum Emulator package.\n");
    if (argc<2) {
       usage:
       puts ("\nUsage: Z802TAP [options]  inputfile[.Z80]  [outputfile[.TAP]]\n");
       puts ("Options are:  /u                  Do not compress the blocks");
       puts ("              /s screen[.SCR]     Add loading screen 'screen.SCR'\n");
       puts ("þ The .TAP file created contains the snapshot in a loadable format. It can be");
       puts ("  written to tape using TAP2TAPE and loaded into an ordinary Spectrum (128),");
       puts ("  but can of course also be loaded into the emulator.");
       puts ("þ Z802TAP will not convert SamRam snapshots.\n");
       exit(1);
    }
    for (i=0;i<8;i++) specmem[i]=(unsigned char *)malloc(16384);
    temp=(unsigned char *)malloc(60000);
    if (temp==NULL) {
       puts ("Error - not enough memory.");
       for (i=0;i<8;i++) printf ("%p\n",specmem[i]);
       exit (1);
    }
    i=1;
    while ((i<argc)&&((argv[i][0]=='/')||(argv[i][0]=='-'))) {
          if (argv[i][2]) {
             errcmd:
             puts ("\007Error on command line!\n");
             goto usage;
          }
          if ((argv[i][1]&0xDF)=='U')
             compr=0;
          else if ((argv[i][1]&0xDF)=='S') {
               if (scr) goto errcmd;
               i++;
               subext(argv[i],".SCR");
               scr=open(filename,O_RDWR+O_BINARY);
          } else goto errcmd;
          i++;
    }
    if (i==argc) goto errcmd;
    subext(argv[i++],".Z80");
    fin=open(filename,O_RDWR+O_BINARY);
    if (fin==-1) {
       printf ("Error - file %s not found\n",filename);
       exit(1);
    }
    if (i>=argc) i--;
    subext(argv[i++],".TAP");
    //fout=_creat(filename,0);
	fout=open(filename,O_CREAT+O_TRUNC+O_WRONLY+O_BINARY,S_IRUSR | S_IWUSR);
    if (fout==-1) {
       printf ("Error - file %s cannot be created\n",filename);
       exit(1);
    }
    if (i<argc) goto usage;
    if (read(fin,&header,30)!=30) {
       errz80:
       puts ("\n\nError in .Z80 file.");
       exit (1);
    }
    if (header.pcreg!=0) {  /* prior to v2.0 */
       header.hmode=0;
       printf ("Processing: ......\rProcessing: ");
       if ((header.flagbyte==0xFF)||((header.flagbyte&0x20)==0)) {  /* old.. */
          if (read(fin,specmem[5],16384)!=16384) goto errz80;
          printf ("o");
          if (read(fin,specmem[1],16384)!=16384) goto errz80;
          printf ("o");
          if (read(fin,specmem[2],16384)!=16384) goto errz80;
          printf ("o");
       } else {   /* not-so-old */
         i=read(fin,temp,49152L);
         printf ("ooo");
         memmove(temp+59000L-i,temp,i);
         if (unpack(temp+59000L-i,temp,49152L)!=i-4) goto errz80;
         memcpy(specmem[5],temp,16384);
         memcpy(specmem[1],temp+16384,16384);
         memcpy(specmem[2],temp+32768L,16384);
       }
    } else {
      if (read(fin,&header.length,2)!=2) goto errz80;
      if ((header.length!=23)&&(header.length!=54)) goto errz80;
      if (read(fin,&header.pcreg2,header.length)!=(header.length)) goto errz80;
      header.pcreg=header.pcreg2;
      if ((header.length==23)&&(header.hmode>=3)) header.hmode++;
      if (header.hmode==2) {
         puts ("\n\nError - Won't convert SamRam snapshots! (Change hardware mode in emulator)");
         exit (1);
      } else if (header.hmode<4)
          printf ("Processing: ......");
      else {
            if (header.outstate&8) printf ("\007WARNING: Program may not load correctly. Try snapping it at a different point.\n\n");
            printf ("Processing: ................");
      }
      printf ("\rProcessing: ");
      if (read(fin,&block,3)!=3) goto errz80;
      do {
         if (read(fin,temp,block.length)!=block.length) goto errz80;
         printf ("o");
         if (unpack(temp,specmem[block.page-3],16384)!=block.length) goto errz80;
      } while (read(fin,&block,3)==3);
    }
    if (header.hmode<4) {
       for (i=0;i<16;i++) header.soundreg[i]=0;
       header.outstate=16;
    }

    write(fout,&LOADER,425);
    tapehdr.type=3;
    for (i=0;i<10;i++) tapehdr.fnam[i]=' ';
    for (i=j=0;filename[i];i++) {
        if (filename[i]=='\\') j=i+1;
    }
    for (i=0;filename[j]&&(filename[j]!='.');j++,i++) tapehdr.fnam[i]=filename[j];
    tapehdr.b2='G';
    tapehdr.b3='L';
    if (scr) {
       tapehdr.fnam[9]='S';
       tapehdr.length=6912;
       tapehdr.b0=11;
       writetap((unsigned char *)&tapehdr,0,17);
       if (read(scr,temp,7000)!=6912) {
          close(fout);
          puts ("\n\nError - Screen snapshot not found or not exactly 6912 bytes long!");
          exit(1);
       }
       writetap(temp,0xFF,6912);
    }
    tapehdr.fnam[9]='0';
    for (i=0;i<8;i++) {
        if (((header.hmode>=4)||(i==1)||(i==2))&&(i!=5)) {
           printf ("*\b");
           if (compr) j=pack(specmem[i]); else j=16384;
           if (j==16384) memcpy(temp,specmem[i],16384);
           tapehdr.length=j;
           if (header.hmode>=4)
              tapehdr.b0=i;
           else
               tapehdr.b0=i+7;
           printf ("o");
           writetap((unsigned char *)&tapehdr,0,17);
           writetap(temp,0xFF,j);
           tapehdr.fnam[9]++;
        }
    }

    tapehdr.length=16384;
    tapehdr.b0=10;
    tapehdr.b1=header.ireg;
    printf ("o");
    writetap((unsigned char *)&tapehdr,0,17);
    memcpy(temp,specmem[5],16384);
    for (i=0;i<32;i++) {
        temp[i+0x15e0]=loadline1[i];
        temp[i+0x16e0]=loadline2[i];
    }
    #define copyword(i,j) temp[i]=j; temp[i+1]=(j>>8);
    temp[0x15e9]=header.outstate;
    temp[0x15f8]=(header.flagbyte>>1)&7;
    copyword(0x16e2,header.bcreg);
    copyword(0x16e5,header.dereg);
    copyword(0x16e8,header.hlreg);
    copyword(0x16ec,header.ixreg);
    temp[0x16f1]=((header.rreg-8)&0x7f)|((header.flagbyte&1)<<7);
    copyword(0x16f8,header.spreg);
    if ((header.imode&3)==2) temp[0x16fb]=0x5e;
    if (header.iff1==0) temp[0x16fc]=0xf3;
    copyword(0x16fe,header.pcreg);
    temp[0x17e4]=0xe0;
    temp[0x17e5]=0x55;
    for (i=0;i<14;i++) temp[0x17e6+i]=header.soundreg[i];
    copyword(0x17f4,header.hlareg);
    copyword(0x17f6,header.bcareg);
    copyword(0x17f8,header.deareg);
    copyword(0x17fa,header.iyreg);
    temp[0x17fc]=header.fareg;
    temp[0x17fd]=header.aareg;
    temp[0x17fe]=header.freg;
    temp[0x17ff]=header.areg;
    writetap(temp,0xFF,16384);
    if (close(fout)) {
       puts ("\n\nError writing to .TAP file");
       exit (1);
    }
    puts (",  Done.");
    return(0);
}


