#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Binary representation of x^4 + x + 1
#define MODULUS 0b10011  

// S-Box
const unsigned char sBox[16] = {
    0x9, 0x4, 0xA, 0xB,
    0xD, 0x1, 0x8, 0x5,
    0x6, 0x2, 0x0, 0x3,
    0xC, 0xE, 0xF, 0x7};

//Inverse S-Box
const unsigned char InvSBox[16] = {
    0XA, 0x5, 0x9, 0xB,
    0x1, 0x7, 0x8, 0xF,
    0x6, 0x0, 0x2, 0x3,
    0xC, 0x4, 0xD, 0xE};


 // Substitute nibbles for 8 bits input
char SubNib(char in)
{
    unsigned char nib1 = (in & 0XF0) >> 4;
    unsigned char nib2 = in & 0X0F;
    return (sBox[nib1] << 4) | sBox[nib2];
}


// Rotate nibbles
char RotNib(char in)
{
    char right = in & 0X0F;
    right = right << 4 ;
    char left = (in & 0XF0) >> 4;
    return right | left;
}


// Function to perform multiplication in GF(2^4)
char mult_GF2_4(char a, char b)
{
    char result = 0;

    for (int i = 0; i < 4; ++i)
    {
        if (b & 1)
        {
            result ^= a; // XOR operation
        }

        int highBitSet = a & 0b1000;
        a <<= 1;

        if (highBitSet)
        {
            a ^= MODULUS;
        }

        b >>= 1;
    }

    return result;
}


// Multiply 2 matrices in GF(2^4)
void multMatrices(short firstMatrix[2][2], short secondMatrix[2][2], short result[2][2])
{
    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            result[i][j] = 0;

            for (int k = 0; k < 2; ++k)
            {
                result[i][j] ^= mult_GF2_4(firstMatrix[i][k], secondMatrix[k][j]);
            }
        }
    }
}

// Display matrix
void displayMatrix(short matrix[2][2])
{
    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}

// Nibble substitution function for 16-bit input
unsigned short substituteNibbles(unsigned short input)
{
    // Masking to separate the two 4-bit nibbles
    unsigned short firstNibble = input & 0x000F;
    unsigned short secondNibble = input & 0x00F0;
    unsigned short thirdNibble = input & 0x0F00;
    unsigned short fourthNibble = input & 0xF000;

    // Perform nibble substitution for each nibble
    unsigned char firstSubstitutedNibble = sBox[firstNibble];
    unsigned char secondSubstitutedNibble = sBox[secondNibble >> 4];
    unsigned char thirdSubstitutedNibble = sBox[thirdNibble >> 8];
    unsigned char fourthSubstitutedNibble = sBox[fourthNibble >> 12];
    
    // Combine the substituted nibbles back into a 16-bit result
    return firstSubstitutedNibble | secondSubstitutedNibble << 4 | thirdSubstitutedNibble << 8 | fourthSubstitutedNibble << 12;
}

// Inverse nibble substitution function for 16-bit input
unsigned short InverseSubstituteNibbles(unsigned short input)
{
    // Masking to separate the two 4-bit nibbles
    unsigned short firstNibble = input & 0x000F;
    unsigned short secondNibble = input & 0x00F0;
    unsigned short thirdNibble = input & 0x0F00;
    unsigned short fourthNibble = input & 0xF000;

    // Perform nibble substitution for each nibble
    unsigned char firstSubstitutedNibble = InvSBox[firstNibble];
    unsigned char secondSubstitutedNibble = InvSBox[secondNibble >> 4];
    unsigned char thirdSubstitutedNibble = InvSBox[thirdNibble >> 8];
    unsigned char fourthSubstitutedNibble = InvSBox[fourthNibble >> 12];
    // Combine the substituted nibbles back into a 16-bit result
    return firstSubstitutedNibble | secondSubstitutedNibble << 4 | thirdSubstitutedNibble << 8 | fourthSubstitutedNibble << 12;
}

// Shift rows function for 16-bit input
unsigned short shiftRows(unsigned short input)
{
    unsigned short firstNibble = (input & 0x000F) << 8;
    unsigned short thirdNibble = (input & 0x0F00) >> 8;
    unsigned short out = (input & 0xF0F0) | firstNibble | thirdNibble;
    return out;
}

unsigned short DEC(unsigned short cipher, unsigned short key)
{
    unsigned short w0 = (key & 0XFF00) >> 8;
    unsigned short w1 = key & 0X00FF;
    unsigned short w2 = w0 ^ 0b10000000 ^ SubNib(RotNib(w1));
    unsigned short w3 = w1 ^ w2;
    unsigned short w4 = w2 ^ 0b00110000 ^ SubNib(RotNib(w3));
    unsigned short w5 = w3 ^ w4;
    unsigned short key0 = key;
    unsigned short key1 = (w2 << 8) | w3;
    unsigned short key2 = (w4 << 8) | w5;

    // Add Round Key 0
    unsigned short addround0 = cipher ^ key2;

    // Perform nibble substitution for the entire block
    unsigned short shift1 = shiftRows(addround0);
    unsigned short s_box1 = InverseSubstituteNibbles(shift1);

    unsigned short addround1 = s_box1 ^ key1;

    short mix[2][2] = {{9, 2}, {2, 9}};
    short shiftMatrix[2][2] = {{(addround1 & 0XF000) >> 12, (addround1 & 0X00F0) >> 4}, {(addround1 & 0X0F00) >> 8, (addround1 & 0X000F)}};
    short mixedMatrix[2][2];

    multMatrices(mix, shiftMatrix, mixedMatrix);

    unsigned short mixed1 = mixedMatrix[0][0] << 12 | mixedMatrix[1][0] << 8 | mixedMatrix[0][1] << 4 | mixedMatrix[1][1];

    unsigned short shift2 = shiftRows(mixed1);
    unsigned short s_box2 = InverseSubstituteNibbles(shift2);

    unsigned short addround2 = s_box2 ^ key0;
    unsigned short plaintext = addround2;

    return plaintext;
}

unsigned short ENC(unsigned short plaintext, unsigned short key)
{
    // Key generation
    unsigned short w0 = (key & 0XFF00) >> 8;
    unsigned short w1 = key & 0X00FF;
    unsigned short w2 = w0 ^ 0b10000000 ^ SubNib(RotNib(w1));
    unsigned short w3 = w1 ^ w2;
    unsigned short w4 = w2 ^ 0b00110000 ^ SubNib(RotNib(w3));
    unsigned short w5 = w3 ^ w4;
    unsigned short key0 = key;
    unsigned short key1 = (w2 << 8) | w3;
    unsigned short key2 = (w4 << 8) | w5;

    // Add Round Key 0
    unsigned short addround0 = plaintext ^ key0;

    ////
    printf("add round0 = 0x%04X\n", addround0);

    // Perform nibble substitution for the entire block
    unsigned short s_box1 = substituteNibbles(addround0);

    ////
    printf("substituteNibbles = 0x%04X\n", s_box1);

    unsigned short shift1 = shiftRows(s_box1);
    ////
    printf("shiftRows = 0x%04X\n", shift1);

    // Mix columns
    short mix[2][2] = {{1, 4}, {4, 1}};
    short shiftMatrix[2][2] = {{(shift1 & 0XF000) >> 12, (shift1 & 0X00F0) >> 4}, {(shift1 & 0X0F00) >> 8, (shift1 & 0X000F)}};
    short mixedMatrix[2][2];
    multMatrices(mix, shiftMatrix, mixedMatrix);
    unsigned short mixed1 = mixedMatrix[0][0] << 12 | mixedMatrix[1][0] << 8 | mixedMatrix[0][1] << 4 | mixedMatrix[1][1];
    ////
    printf("mixcolumns = 0x%04X\n", mixed1);

    // Add round 1 key
    unsigned short addround1 = mixed1 ^ key1;

    ////
    printf("addround1 = 0x%04X\n", addround1);


    // Final Round
    unsigned short s_box2 = substituteNibbles(addround1);
    ////
    printf("substituteNibbles = 0x%04X\n", s_box2);

    unsigned short shift2 = shiftRows(s_box2);
    ////
    printf("shift2 = 0x%04X\n", shift2);

    unsigned short addround2 = shift2 ^ key2;
    ////
    printf("addround2 = 0x%04X\n", addround2);

    unsigned short cipher = addround2;

    return cipher;
}

// Function to convert a hexadecimal string to binary and return as unsigned short
unsigned short hexStringToBinary(const char* hexString) {
    unsigned short binaryResult = 0;

    // Iterate through each character in the string
    for (int i = 0; hexString[i] != '\0'; ++i) {
        binaryResult <<= 4; // Left shift the current value by 4 bits

        // Convert hexadecimal character to integer
        if (hexString[i] >= '0' && hexString[i] <= '9') {
            binaryResult += hexString[i] - '0';
        } else if (hexString[i] >= 'A' && hexString[i] <= 'F') {
            binaryResult += hexString[i] - 'A' + 10;
        } else if (hexString[i] >= 'a' && hexString[i] <= 'f') {
            binaryResult += hexString[i] - 'a' + 10;
        } else {
            // Handle invalid characters in the string
            fprintf(stderr, "Invalid hexadecimal character: %c\n", hexString[i]);
            return 0; // or any other error handling strategy
        }
    }

    return binaryResult;
}

// Main function for demonstration
int main(int argc, char* argv [])
{
    char* op = argv[1];
    char* GlobalKey = argv[2];
    char* GlobalData = argv[3];

    unsigned short text = hexStringToBinary(GlobalData);
    unsigned short key = hexStringToBinary(GlobalKey);

    // // Example input 16-bit block
    // unsigned short text = 0b1101011100101000; // Replace with your input value
    // unsigned short key = 0b0100101011110101;
    // printf(argv);
    if (strcmp(op, "ENC") == 0)
    {
    unsigned short cipher = ENC(text, key);
    printf("0x%04X\n", cipher);
    }

    if(strcmp(op, "DEC") == 0)
    {
    unsigned short plaintext = DEC(text, key);
    printf("0x%04X\n", plaintext); 
    }

    // Display the results
    
    

    return 0;
}