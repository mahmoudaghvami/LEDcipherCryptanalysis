#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int LED = 96; // to use, change this to the key size wanted, assumed to be multiple of 4.
const unsigned char MixColMatrix[4][4] =
{
	{ 4,  1, 2, 2 },
	{ 8,  6, 5, 6 },
	{ 11,14,10, 9 },
	{ 2,  2,15,11 },
};

const unsigned char revMixColMatrix[4][4] =
{
	{ 12,12,13,4 },
	{ 3,8,4,5 },
	{ 7,6,2,14 },
	{ 13,9,9,13 },
};

const unsigned char sbox[16] = { 12, 5, 6, 11, 9, 0, 10, 13, 3, 14, 15, 8, 4, 7, 1, 2 };
const unsigned char revsbox[16] = { 5, 14, 15, 8, 12, 1, 2, 13, 11, 4, 6, 3, 0, 7, 9, 10 };
const unsigned char WORDFILTER = 0xF;
int DEBUG = 0;


void AddKey(unsigned char state[4][4], unsigned char* keyBytes, int step)
{
	int i, j;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
		{
			state[i][j] ^= keyBytes[((4 * i) + j + (step * 16)) % (LED / 4)];
		}
			
}

void AddConstants(unsigned char state[4][4], int r) //RC = round constant
{
	const unsigned char RC[48] = {
		0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3E, 0x3D, 0x3B, 0x37, 0x2F,
		0x1E, 0x3C, 0x39, 0x33, 0x27, 0x0E, 0x1D, 0x3A, 0x35, 0x2B,
		0x16, 0x2C, 0x18, 0x30, 0x21, 0x02, 0x05, 0x0B, 0x17, 0x2E,
		0x1C, 0x38, 0x31, 0x23, 0x06, 0x0D, 0x1B, 0x36, 0x2D, 0x1A,
		0x34, 0x29, 0x12, 0x24, 0x08, 0x11, 0x22, 0x04
	};

	state[1][0] ^= 1;
	state[2][0] ^= 2;
	state[3][0] ^= 3;

	state[0][0] ^= (LED >> 4) & 0xf;
	state[1][0] ^= (LED >> 4) & 0xf;
	state[2][0] ^= LED & 0xf;
	state[3][0] ^= LED & 0xf;

	unsigned char tmp = (RC[r] >> 3) & 7;
	state[0][1] ^= tmp;
	state[2][1] ^= tmp;
	tmp = RC[r] & 7;
	state[1][1] ^= tmp;
	state[3][1] ^= tmp;
}

void SubCell(unsigned char state[4][4])
{
	int i, j;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			state[i][j] = sbox[state[i][j]];
}

void revSubCell(unsigned char state[4][4])
{
	int i, j;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			state[i][j] = revsbox[state[i][j]];
}

void ShiftRow(unsigned char state[4][4])
{
	int i, j;
	unsigned char tmp[4];
	for (i = 1; i < 4; i++) //i=1 ya 2 ya 3
	{
		for (j = 0; j < 4; j++)
			tmp[j] = state[i][j];
		for (j = 0; j < 4; j++)
			state[i][j] = tmp[(j + i) % 4];
	}
}

void revShiftRow(unsigned char state[4][4])
{
	int i, j;
	unsigned char tmp[4];
	for (i = 1; i < 4; i++) //i=1 ya 2 ya 3
	{
		for (j = 0; j < 4; j++)
			tmp[j] = state[i][j];
		//first row nothing
		if (i == 1)
		{
			state[i][0] = tmp[3];
			state[i][1] = tmp[0];
			state[i][2] = tmp[1];
			state[i][3] = tmp[2];
		}
		if (i == 2)
		{
			state[i][0] = tmp[2];
			state[i][1] = tmp[3];
			state[i][2] = tmp[0];
			state[i][3] = tmp[1];
		}
		if (i == 3)
		{
			state[i][0] = tmp[1];
			state[i][1] = tmp[2];
			state[i][2] = tmp[3];
			state[i][3] = tmp[0];
		}
	}
}

unsigned char FieldMult(unsigned char a, unsigned char b)
{
	const unsigned char ReductionPoly = 0x3;
	unsigned char x = a, ret = 0;
	int i;
	for (i = 0; i < 4; i++) {
		if ((b >> i) & 1) ret ^= x;
		if (x & 0x8) {
			x <<= 1;
			x ^= ReductionPoly;
		}
		else x <<= 1;
	}
	return ret & WORDFILTER;
}

void MixColumn(unsigned char state[4][4])
{
	int i, j, k;
	unsigned char tmp[4];
	for (j = 0; j < 4; j++) {
		for (i = 0; i < 4; i++) {
			unsigned char sum = 0;
			for (k = 0; k < 4; k++)
				sum ^= FieldMult(MixColMatrix[i][k], state[k][j]);
			tmp[i] = sum;
		}
		for (i = 0; i < 4; i++)
			state[i][j] = tmp[i];
	}
}

void revMixColumn(unsigned char state[4][4])
{
	int i, j, k;
	unsigned char tmp[4];
	for (j = 0; j < 4; j++)
	{
		for (i = 0; i < 4; i++)
		{
			unsigned char sum = 0;
			for (k = 0; k < 4; k++)
				sum ^= FieldMult(revMixColMatrix[i][k], state[k][j]);
			tmp[i] = sum;
		}
		for (i = 0; i < 4; i++)
			state[i][j] = tmp[i];
	}
}

void LED_enc(unsigned char* input, const unsigned char* userkey, int ksbits)
{
	unsigned char state[4][4];
	unsigned char keyNibbles[32]; //hadaksar 128 bit key dashtim dige
	int i;
	for (i = 0; i < 16; i++) {
		if (i % 2) state[i / 4][i % 4] = input[i >> 1] & 0xF;
		else state[i / 4][i % 4] = (input[i >> 1] >> 4) & 0xF;
	}

	memset(keyNibbles, 0, 32);//hame 32 ta niblle ra aval sefr mikone
	for (i = 0; i < ksbits / 4; i++) {
		if (i % 2) keyNibbles[i] = userkey[i >> 1] & 0xF;
		else keyNibbles[i] = (userkey[i >> 1] >> 4) & 0xF;
	}
	LED = ksbits;
	int Rounds;
	Rounds = 4;
	//Rounds = 32;
	//if(ksbits > 64)
	//	Rounds = 48;

	int j;
	AddKey(state, keyNibbles, 0);
	for (i = 0; i < Rounds / 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			AddConstants(state, i * 4 + j);
			SubCell(state);
			ShiftRow(state);
			MixColumn(state);
		}
		AddKey(state, keyNibbles, i + 1);
	}

	/*printf("\nalooo:\n");
	for (int i = 0; i <4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			printf("%d\t", state[i][j]);
		}
		printf("\n");
	}*/

	for (i = 0; i < 8; i++)
		input[i] = ((state[(2 * i) / 4][(2 * i) % 4] & 0xF) << 4) | (state[(2 * i + 1) / 4][(2 * i + 1) % 4] & 0xF);
}


void LED_dec(unsigned char* input, const unsigned char* userkey, int ksbits)
{
	unsigned char state[4][4];
	unsigned char keyNibbles[32]; //hadaksar 128 bit key dashtim dige
	int i;
	for (i = 0; i < 16; i++) {
		if (i % 2) state[i / 4][i % 4] = input[i >> 1] & 0xF;
		else state[i / 4][i % 4] = (input[i >> 1] >> 4) & 0xF;
	}
	
	memset(keyNibbles, 0, 32);//hame 32 ta niblle ra aval sefr mikone
	for (i = 0; i < ksbits / 4; i++) {
		if (i % 2) keyNibbles[i] = userkey[i >> 1] & 0xF;
		else keyNibbles[i] = (userkey[i >> 1] >> 4) & 0xF;
	}
	LED = ksbits;
	int Rounds;
	Rounds = 4;
	//Rounds = 32;
	//if(ksbits > 64)
	//	Rounds = 48;

	int j;
	AddKey(state, keyNibbles, 1);
	for (i = (Rounds / 4)-1 ; i >=0 ; i--)
	{
		for (j = 3; j >=0; j--)
		{
			revMixColumn(state);
			revShiftRow(state);
			revSubCell(state);
			AddConstants(state, i * 4 + j);
		}
		AddKey(state, keyNibbles,0);
	}

	/*printf("\nalooo2:\n");
	for (int i = 0; i <4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			printf("%d\t", state[i][j]);
		}
		printf("\n");
	}*/

	for (i = 0; i < 8; i++)
		input[i] = ((state[(2 * i) / 4][(2 * i) % 4] & 0xF) << 4) | (state[(2 * i + 1) / 4][(2 * i + 1) % 4] & 0xF);
}

void Random_generator(unsigned char *input, unsigned char *key,int kbits)
{
	srand((unsigned)time(0)); 
	for(int i = 0; i < 8; i++)  input[i] = rand() & 0xff; 
	for(int i = 0; i < kbits/8 ; i++)key[i] = rand() & 0xff;
}

int main(int argc, char*argv[])
{
	int kbitso = 96;
	/*unsigned char input[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	unsigned char key[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };*/
	unsigned char input[8] = { 12, 34, 56, 4, 0, 0, 0, 0 };
	unsigned char key[16] = { 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120,120 };
	//Random_generator(input,key,kbitso);
	printf("Key = "); for (int i = 0; i < kbitso / 8; i++) printf("%02x", key[i]); printf("\n");
	printf("Plain = "); for (int i = 0; i < 8; i++) printf("%02x", input[i]); printf("\n");
	LED_enc(&input[0], &key[0], kbitso);
	printf("Cipher = "); for (int i = 0; i < 8; i++) printf("%02x", input[i]); printf("\n\n");
	LED_dec(&input[0], &key[0], kbitso);
	printf("decrypted cipher = "); for (int i = 0; i < 8; i++) printf("%02x", input[i]); printf("\n\n");
	getchar();
	return 0;
}

