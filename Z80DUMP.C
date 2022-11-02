/*
    This file is part of the registered Spectrum emulator package 'Z80'
    version 3.02, and may not be distributed.  You may use this source for
    other PC based Spectrum or Z80 emulators only after permission.  It is
    however permitted to use this source file or parts thereof for Spectrum
    or Z80 emulators on non-PC based machines, provided that the source is
    acknowledged.

                                                      Gerton Lunter, 29/10/94
*/

//soluciona el problema de la lectura de short ints de fichero z80, el puntero de lectura se desalinea empezando en byte par
#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

#include <stdio.h>
#include <stdlib.h>

#define word unsigned short int
#define byte unsigned char

struct header {
    byte ra,rf;                 /* version 1 stuff */
    word rbc,rhl,rpc,rsp;
    byte ri,rr,rr_bit7;
    word rde,rbca,rdea,rhla;
    byte raa,rfa;
    word riy,rix;
    byte iff,iff2,imode;
    word length;                /* version 2 stuff */
    word rpc2;
    byte rommod;
    byte hstate;
    byte if1flg;
    byte mflag;
    byte sreg;
    byte sstate[16];
    word tstates;               /* version 3 stuff */
    byte viertel;
    byte zero;
    byte discflg;
    byte m128flg;
    byte rompage1,rompage2;
    word udef_joy[5];
    word udef_keys[5];
    byte disctype;
    byte discinhibit;
    byte discinhibitflg;
} buffer;

struct blockheader {
    word length;
    byte page;
} blockhdr;


word i;
byte ver,j,rommod,verbose=0;
byte *buf1,*buf2;
FILE *fin;


word unpack(byte*,byte*,word);
int main(int,char**);


word unpack(inp,outp,size)
byte *inp,*outp;
word size;
{
    register word incount=0,outcount=0;
    word i;
    byte j;
    do {
       if ((inp[0]==0xED)&&(inp[1]==0xED)) {
            i=inp[2];
            j=inp[3];
            inp+=4;
            incount+=4;
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


int main(teller,str)
int teller;
char **str;
{
    printf ("Z80DUMP - .Z80 file inspection utility - (c) 1994 G.A. Lunter - version 3.0\n");
    printf ("          This program may not be distributed.\n");
    printf ("          It is part of the registered Spectrum Emulator package.\n\n\n");
    if ((teller!=2)&&(teller!=3)) {
	   puts ("Syntax: Z80DUMP  file[.z80]  [-v]");
       puts ("\nWhen -v is specified, the entire memory contents is displayed.\n");
	   return(1);
    }
    if (teller==3) {if ((str[2][1]&0xDF)=='V') verbose=1;}
    fin=fopen(str[1],"rb");
    if (fin==NULL) {
	   for (i=0;str[1][i];i++) {}
	   str[1][i++]='.';          /* dirty */
	   str[1][i++]='Z';
	   str[1][i++]='8';
	   str[1][i++]='0';
	   str[1][i]=0;              /* but it works */
	   fin=fopen(str[1],"rb");
    }
    if (fin==NULL) {
	   printf ("File %s not found.\n",str[1]);
	   return(1);
    }
    if (fread(&buffer,1,30,fin)!=30) {
       errfile:
	   printf ("Error while reading file %s.\n",str[1]);
	   return(1);
    }
    ver=0;
    printf ("Header of file %s\n",str[1]);
    if (buffer.rpc==0) {
        if (fread(&(buffer.length),1,2,fin)!=2) goto errfile;
        if (verbose) printf ("Length of 2nd header block: %d bytes\n",buffer.length);
        switch (buffer.length) {
        case 21:
        case 23:
		     printf ("(Version 2.0 .Z80 file)\n\n");
             ver=2;
             break;
        case 54:
             printf ("(Version 3.0 .Z80 file)\n\n");
             ver=3;
             break;
        default:
	         printf ("\nUnsupported version of .Z80 file, or .Z80 file corrupt\n\n");
	         return(1);
	    }
        if (fread(&(buffer.rpc2),1,buffer.length,fin)!=buffer.length) goto errfile;
        buffer.rpc=buffer.rpc2;
    } else
	  printf ("(Version 1.45 or earlier .Z80 file)\n\n");
    if (buffer.rr_bit7==0xff) buffer.rr_bit7=1;
    if (ver==0) {
       printf ("Data compression             ");
	   if (buffer.rr_bit7&32) printf ("Active\n"); else printf ("No compression\n");
       printf ("SamRom                       Switched ");
	   if (buffer.rr_bit7&16) printf ("on\n"); else printf ("off\n");
    } else {
	  printf ("Selected hardware            ");
      if ((ver==2)&&(buffer.rommod>=3)) buffer.rommod++;
      switch (buffer.rommod) {
	    case 0: printf ("Normal Spectrum\n");
		    break;
	    case 1: printf ("Normal Spectrum + If.1\n");
		    printf ("Interface 1 ROM              ");
		    if (buffer.if1flg) printf ("paged\n"); else printf ("inactive\n");
		    break;
	    case 2: printf ("Normal Spectrum + If.1 + SamRam\n");
		    printf ("Interface 1 ROM              ");
		    if (buffer.if1flg) printf ("paged\n"); else printf ("inactive\n");
		    printf ("State of OUT-port 31         ");
		    for (i=0;i<8;i++) {
			    printf ("%d",2*i+((buffer.hstate>>i)&1));
			    if (i!=7) printf (","); else printf ("\n");
		    }
		    break;
        case 3: printf ("Normal Spectrum + M.G.T. Interface\n");
            printf ("MGT Interface type           ");
            if (buffer.disctype==0) printf ("Disciple (Epson printer)\n");
               else if (buffer.disctype==1) printf ("Disciple (HP PLC printer)\n");
               else if (buffer.disctype==16) printf ("+D (Epson printer)\n");
               else if (buffer.disctype==17) printf ("+D (HP PLC printer)\n");
               else if (buffer.disctype==8) printf ("UNIDos\n");
               else printf ("Unknown type\n");
            printf ("MGT Interface ROM            ");
            if (buffer.discflg) printf ("paged\n"); else printf ("inactive\n");
            printf ("Inhibit button               ");
            if (buffer.discinhibit) printf ("IN\n"); else printf ("out\n");
            printf ("Inhibit flag                 ");
            if (buffer.discinhibitflg) printf ("Active\n"); else printf ("inactive\n");
            break;
        case 4:
        case 5:printf ("Spectrum 128");
		    if (buffer.rommod==5) {
			   printf (" + If.1");
			   printf("\nInterface 1 ROM              ");
			   if(buffer.if1flg) printf ("Paged"); else printf ("inactive");
		    }
		    printf ("\nState of OUT-port 7ffd       #%02x\n",buffer.hstate);
		    printf ("State of OUT-port fffd       #%02x\n",buffer.sreg);
		    printf ("Sound chip registers         ");
            j=16;
            if (buffer.length==21) j=14;
		    for (i=0;i<j;i++) {
			    printf ("%02x",buffer.sstate[i]);
			    if (i==j-1) printf ("\n"); else printf (",");
		    }
		    break;
	    }
	    printf ("R-register emulation         ");
	    if (buffer.mflag&1) printf ("On\n"); else printf ("Off\n");
	    printf ("Ldir emulation               ");
	    if (buffer.mflag&2) printf ("On\n"); else printf ("Off\n");
    }
    printf ("Issue 2 emulation            ");
    if (buffer.imode&4) printf ("On\n"); else printf ("Off\n");
    printf ("Double interrupt frequency   ");
    if (buffer.imode&8) printf ("On\n"); else printf ("Off\n");
    printf ("Video synchronisation        ");
    switch (buffer.imode&48) {
	  case 16:printf ("High\n");
		break;
	  case 48:printf ("Low\n");
		break;
	  default:printf ("Normal\n");
    }
    printf ("Joystick emulation           ");
    switch (buffer.imode&192) {
	  case 0:printf ("Cursor\n");
		break;
	  case 64:printf ("Kempston\n");
		break;
	  case 128:if (ver==2) printf ("Sinclair 1\n"); else {
           printf ("User defined\n");
        }
        break;
	  case 192:printf ("Sinclair 2\n");
    }
    printf ("Border colour                %d\n",(buffer.rr_bit7&14)>>1);
    if (ver==3) {
       printf ("User defined joystick keys:  ");
       for (i=0;i<5;i++) {
           j=buffer.udef_keys[i];
           switch (j) {
                  case ']': printf ("SymShft"); break;
                  case '[': printf ("Shft"); break;
                  case '/': printf ("Enter"); break;
                  case '\\':printf ("Space"); break;
                  default: printf ("%c",j);
           }
           if (i==4) printf ("\n"); else printf (",");
       }
       printf ("ULA time                     ");
       if (buffer.tstates>17472) buffer.tstates=17472;
       if (buffer.viertel>=4)
          printf ("Wrong Format!\n"); else
          printf ("%ld T states after interrupt\n",17472L*(long)(((buffer.viertel+1)&3)+1)-(long)buffer.tstates);
       printf ("Memory from 0 to 8191:       ");
       if (buffer.rompage1) printf ("rom\n"); else printf("RAM\n");
       printf ("Memory from 8192 to 16383:   ");
       if (buffer.rompage2) printf ("rom\n"); else printf("RAM\n");
       if (buffer.zero!=0) printf ("Zero byte is not zero (#%02x)  This file must be from Carlo's Speculator!\n",buffer.zero);
       if (buffer.m128flg) printf ("Multiface 128 rom/ram paged!  Snapshot will probably crash.\n");
    }
    printf ("\nInterrupt status      IM%d, ",buffer.imode&3);
    if (buffer.iff) printf ("EI"); else printf ("DI");
    printf (" (IFF2 = ");
    if (buffer.iff2) printf ("EI)\n"); else printf ("DI)\n");
    printf ("Interrupt register    #%02x\n",buffer.ri);
    printf ("Refresh register      #%02x (bit 8 is %d)\n",(buffer.rr&0x7f)|((buffer.rr_bit7<<7)&0x80),buffer.rr_bit7&1);
    printf ("Program counter       #%04x                Stack pointer         #%04x\n",buffer.rpc,buffer.rsp);
    printf ("Register A, F         #%02x, #%02x             Register A', F'       #%02x, #%02x\n",buffer.ra,buffer.rf,buffer.raa,buffer.rfa);
    printf ("Register BC           #%04x                Register BC'          #%04x\n",buffer.rbc,buffer.rbca);
    printf ("Register DE           #%04x                Register DE'          #%04x\n",buffer.rde,buffer.rdea);
    printf ("Register HL           #%04x                Register HL'          #%04x\n",buffer.rhl,buffer.rhla);
    printf ("Register IX           #%04x\n",buffer.rix);
    printf ("Register IY           #%04x\n\n",buffer.riy);
    printf ("\n");

    #define printblock(len,adr0) \
        for (i=0;i<len;i+=16) { \
            printf ("#%04x:  ",i+adr0); for (j=0;j<16;j++) { \
                printf ("%02x ",buf2[i+j]); \
                if (j==7) printf (" "); if (j==15) printf ("\n"); } } \
        printf ("\n");

    if (ver<2) {
         buf1=(unsigned char*)malloc((size_t)(49152L+256));
         buf2=(unsigned char*)malloc((size_t)(49152L+256));
         if (buf2==NULL) {
            notenoughmemory:
            printf ("Not enough memory to load memory image.\n");
            return(1);
         }
         i=fread(buf1,1,49152L+256,fin);
         if (buffer.rr_bit7&32) {
            if (unpack(buf1,buf2,49152L)!=(i-4)) {
               printf ("Error in .Z80 file: block doesn't expand to 48K, file too long, or\nmissing end marker.\n");
               return(1);
            }
            if ((buf1[i-4]!=0)||(buf1[i-3]!=0xED)||(buf1[i-2]!=0xED)||(buf1[i-1]!=0)) {
               printf ("WARNING: End marker not found!  File probably corrupt.\n");
            }
         } else {
           if (i!=49152L) {
              printf ("Error in .Z80 file: uncompressed block not exactly 48K bytes\n");
              return(1);
           }
         }
         if (verbose) {printblock(49152L,16384)} else printf ("48K memory image block OK\n");
    } else {
         buf1=(unsigned char*)malloc(16384+256);
         buf2=(unsigned char*)malloc(16384+256);
         if (buf2==NULL) goto notenoughmemory;
         i=fread(&blockhdr,1,3,fin);
		 //printf("\n i= %d \n", i);
         while (i==3) {
             printf ("Page nr: %2d      Block length: %d:  ",blockhdr.page,blockhdr.length);
             i=fread(buf1,1,blockhdr.length,fin);
             if (i!=blockhdr.length) {
                 printf ("\n\nError in .Z80 file: partial block (length: %d) found\n",i);
                 return(1);
             }
             if (unpack(buf1,buf2,16384)!=i) {
                 printf ("\n\nError in .Z80 file: block doesn't expand to exactly 16384 bytes\n");
                 return(1);
             }
             printf ("OK\n");
             if (verbose) {printblock(16384,0)};
             i=fread(&blockhdr,1,3,fin);
         }
         if (i!=0) printf ("Error in .Z80 file: partial block header found\n");
    }
    return(0);
}

