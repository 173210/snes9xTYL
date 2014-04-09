/* File PC1DEC.c */
/* written in Borland Turbo C 2.0 on PC */
/* PC1 Cipher Algorithm ( Pukall Cipher 1 ) */
/* By Alexander PUKALL 1991 */
/* free code no restriction to use */
/* please include the name of the Author in the final software */
/* the Key is 256 bits */
/* Tested with Turbo C 2.0 for DOS and Microsoft Visual C++ 5.0 for Win 32 */

/* Note that PC1COD256.c is the encryption routine */
/* PC1DEC256.c is the decryption routine */
/* Only the K zone change in the two routines */
/* You can create a single routine with the two parts in it */

#include "psp.h"

unsigned short ax,bx,cx,dx,si,tmp,x1a2,x1a0[16],res,i,inter,cfc,cfd,compte;
unsigned char cle[32];
short c;
int c1,count;



int code() {
	dx=x1a2+i;
	ax=x1a0[i];
	cx=0x015a;
	bx=0x4e35;
	
	tmp=ax;
	ax=si;
	si=tmp;
	
	tmp=ax;
	ax=dx;
	dx=tmp;
	
	if (ax!=0) {
		ax=ax*bx;
	}
	
	tmp=ax;
	ax=cx;
	cx=tmp;
	
	if (ax!=0) {
		ax=ax*si;
		cx=ax+cx;
	}
	
	tmp=ax;
	ax=si;
	si=tmp;
	ax=ax*bx;
	dx=cx+dx;
	
	ax=ax+1;
	
	x1a2=dx;
	x1a0[i]=ax;
	
	res=ax^dx;
	i=i+1;
	return 0;
}

int assemble() {
	
	x1a0[0]= ( cle[0]*256 )+ cle[1];
	code();
	inter=res;
	
	x1a0[1]= x1a0[0] ^ ( (cle[2]*256) + cle[3] );
	code();
	inter=inter^res;
	
	x1a0[2]= x1a0[1] ^ ( (cle[4]*256) + cle[5] );
	code();
	inter=inter^res;
	
	x1a0[3]= x1a0[2] ^ ( (cle[6]*256) + cle[7] );
	code();
	inter=inter^res;
	
	x1a0[4]= x1a0[3] ^ ( (cle[8]*256) + cle[9] );
	code();
	inter=inter^res;
	
	x1a0[5]= x1a0[4] ^ ( (cle[10]*256) + cle[11] );
	code();
	inter=inter^res;
	
	x1a0[6]= x1a0[5] ^ ( (cle[12]*256) + cle[13] );
	code();
	inter=inter^res;
	
	x1a0[7]= x1a0[6] ^ ( (cle[14]*256) + cle[15] );
	code();
	inter=inter^res;
	
	x1a0[8]= x1a0[7] ^ ( (cle[16]*256) + cle[17] );
	code();
	inter=inter^res;
	
	x1a0[9]= x1a0[8] ^ ( (cle[18]*256) + cle[19] );
	code();
	inter=inter^res;
	
	x1a0[10]= x1a0[9] ^ ( (cle[20]*256) + cle[21] );
	code();
	inter=inter^res;
	
	x1a0[11]= x1a0[10] ^ ( (cle[22]*256) + cle[23] );
	code();
	inter=inter^res;
	
	x1a0[12]= x1a0[11] ^ ( (cle[24]*256) + cle[25] );
	code();
	inter=inter^res;
	
	x1a0[13]= x1a0[12] ^ ( (cle[26]*256) + cle[27] );
	code();
	inter=inter^res;
	
	x1a0[14]= x1a0[13] ^ ( (cle[28]*256) + cle[29] );
	code();
	inter=inter^res;
	
	x1a0[15]= x1a0[14] ^ ( (cle[30]*256) + cle[31] );
	code();
	inter=inter^res;
	
	
	i=0;
	return 0;
}

//decrypt encrypted buffer 'in' in output buffer 'out' 
//'in' & 'out' have to be same size
//using buff_cle
int decrypt(char *in,char *out,int size_in,char *buff_cle) {
	si=0;
	x1a2=0;
	i=0;


	/* ('abcdefghijklmnopqrstuvwxyz012345') is the default password used*/
	/* if the user enter a key < 32 characters, characters of the default */
	/* password will be used */
	
	strcpy((char *)cle,"abcdefghijklmnopqrstuvwxyz012345");
	
	/*printf ("PC1 Cipher 256 bits \nDECRYPT file OUT.BIN to IN.BIN\n");
	printf("Enter a 32 chars password:");
	gets(buff);*/
	
	if (strlen(buff_cle)>32)
	{ count=32; }
	else
	{count=strlen(buff_cle);}
	
	for (c1=0;c1<count;c1++)
	{
	cle[c1]=buff_cle[c1];
	}
	
	/* OUT.BIN is the source file ( crypted )*/
	/* IN.BIN is the destination file ( decrypted ) */
	
	/*if ((in=fopen("out.bin","rb")) == NULL) {printf("\nError reading file OUT.BIN !\n");fin();}
	if ((out=fopen("in.bin","wb")) == NULL) {printf("\nError writing file IN.BIN !\n");fin();}*/
	
	/* the decrypted file is in IN.BIN */
	
	while ( size_in-- ) {/* c contains the byte read in the file */
	  c=*in++;
		assemble();
		cfc=inter>>8;
		cfd=inter&255; /* cfc^cfd = random byte */
	
		/* K ZONE !!!!!!!!!!!!! */
		/* here the mix of c and cle[compte] is after the decryption of c */
	
		c = c ^ (cfc^cfd);
	
		for (compte=0;compte<=31;compte++) {
			/* we mix the plaintext byte with the key */
			cle[compte]=cle[compte]^c;
		}
	
		//fputc(c,out); /* we write the decrypted byte in the file IN.BIN */
		*out++=c;
	}
	/*fclose (in);
	fclose (out);
	fin();*/
	return 0;
}


