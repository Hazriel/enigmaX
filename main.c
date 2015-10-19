// Copyright <Pierre-François Monville>
// ===========================================================================
// 									enigmaX
// permet de chiffrer et de déchiffrer tout fichier donné en paramètre
// le mot de passe demandé au début est hashé puis sert de graine pour le PRNG
// le PRNG permet de fournir une clé unique égale à la longueur du fichier à coder
// ainsi la sécurité est maximale (seule solution, bruteforcer le mot de passe)
// De plus un brouilleur est utilisé, il mélange la table des caractères (ascii)
// en utilisant le PRNG ou en utilisant le keyFile fourni au cas où une faille
// matériel permettrait d'analyser la ram afin d'inverser les xor, le résultat
// obtenu serait toujours illisible.
//
// Can crypt and decrypt any file given in argument. The password asked is hashed
// to be used as a seed for the PRNG. The PRNG gives a unique key
// which has the same length as the source file, thus the security is maximum
// (the only way to break through is by bruteforce). Moreover, a scrambler is used,
// it scrambles the ascii table using the PRNG or the keyFile given to prevent
// an hardware failure allowing ram analysis to invert the xoring process, making
// such an exploit useless.
//
// USAGE : 
//		enigmax [-h | --help] FILE [-s | --standard | KEYFILE]
//
// 		code or decode the given file
//
// 		KEYFILE: 
// 			path to a keyfile that is used to generate the scrambler instead of the password
//
// 		-s --standard : 
// 	 		put the scrambler off
//
// 		-h --help : 
// 			further help
//
// ===========================================================================

/*
TODO:
crypted folders explorer
graphical interface
hidden password (not portable for now)
special option (multi layer's password, hide extension)
 */


/*
	includes
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>


/*
	constants
 */
#define BUFFER_SIZE 16384  //16384


/*
	global variables
 */
static const char *progName;
static const char *fileName;
static char pathToMainFile[1000] = "./";
static char _isADirectory;
static uint64_t seed[2];
static unsigned char scrambleAsciiTable[256];
static unsigned char unscrambleAsciiTable[256] = "";
static char isCrypting = 1;
static char scrambling = 1;
static long numberOfBuffer;


/*
	-static void usage(int status)
	status : expect EXIT_FAILURE or EXIT_SUCCESS code to choose the output stream

	when the program is typed without arguments in terminal it shows the usage
 */
static void usage(int status)
{
	FILE *dest = (status == 0) ? stdout : stderr;

	if(status == 0){
		fprintf(dest,
			"%s(1)\t\t\tcopyright <Pierre-François Monville>\t\t\t%s(1)\n\nNAME\n\t%s -- crypt or decrypt any data\n\nSYNOPSIS\n\t%s [-h | --help] FILE [-s | --standard | KEYFILE]\n\nDESCRIPTION\n\t(FR) permet de chiffrer et de déchiffrer toutes les données entrées en paramètre le mot de passe demandé au début est hashé puis sert de graine pour le PRNG le PRNG permet de fournir une clé unique égale à la longueur du fichier à coderainsi la sécurité est maximale (seule solution, bruteforcer le mot de passe) De plus un brouilleur est utilisé, il mélange la table des caractères (ascii) en utilisant le PRNG ou en utilisant le keyFile fourni au cas où une faille matérielle permettrait d'analyser la ram afin d'inverser les xor, le résultat obtenu serait toujours illisible.\n\t(EN) Can crypt and decrypt any data given in argument. The password asked is hashedto be used as a seed for the PRNG. The PRNG gives a unique keywhich has the same length as the source file, thus the security is maximum(the only way to break through is by bruteforce). Moreover, a scrambler is used,it scrambles the ascii table using the PRNG or the keyFile given to preventan hardware failure allowing ram analysis to invert the xoring process, makingsuch an exploit useless.\n\n\tthe options are as follows:\n\n\t-h | --help\tfurther help.\n\n\t-s | --standard\tput the scrambler on off.\n\nEXIT STATUS\n\tthe %s program exits 0 on success, and anything else if an error occurs.\n\nEXAMPLES\n\tthe command:\n\n\t\t%s file1\n\n\t\tlet you choose between crypting or decrypting then it will prompt for a password that crypt/decrypt file1 as file1x in the same folder, file1 is not modified.\n\n\tthe command:\n\n\t\t%s file2 keyfile1\n\n\tlet you choose between crypting or decrypting, will prompt for the password that crypt/decrypt file2, uses keyfile1 to generate the scrambler then crypt/decrypt file2 as file2x in the same folder, file2 is not modified.\n\n\tthe command:\n\n\t\t%s file3 -s\n\n\tlet you choose between crypting or decrypting, will prompt for a password that crypt/decrypt the file without using the scrambler, resulting in using the unique key only.\n", progName, progName, progName, progName, progName, progName, progName, progName);
	} else{
		fprintf(dest,
			"Usage: %s [-h | --help] FILE [-s | --standard | KEYFILE]\n\n\tcode or decode the given file\n\n\tKEYFILE: \n\t\tpath to a keyfile that is used to generate the scrambler instead of the password\n\n\t-s --standard : \n\t\t put the scrambler off\n\n\t-h --help : \n\t\tfurther help\n", progName);
	}
	exit(status);
}


/*
	-long ceilRound(float numberToBeRounded)
	returned value : the number rounded (ceil form)

	to prevent from importing all math.h for just one function 
	I had to add it myself
*/
long ceilRound(float numberToBeRounded){
	if (numberToBeRounded - (long) numberToBeRounded > 0)
	{
		return (long) numberToBeRounded + 1;
	}
	return (long) numberToBeRounded;
}

/*
	-uint64_t generateNumber(void)
	returned value :  uint64_t number (equivalent to long long but on all OS)

	random number generator
	with the Xorshift+ algorythm which is one of the quickiest PRNG
	it passes the BigCrush test :
	http://en.wikipedia.org/wiki/Xorshift
 */
uint64_t generateNumber()
{
	uint64_t x = seed[0];
	uint64_t const y = seed[1];
	seed[0] = y;
	x ^= x << 23; // a
	x ^= x >> 17; // b
	x ^= y ^ (y >> 26); // c
	seed[1] = x;
	return x + y;
}


/*
	-void hash(char* password)
	password : a string which is the password typed by the user

	simple function that hashes a string into numbers
	we don't have to worry about security here because it is just to transform the password into numbers
	that populate the seeds of the RNG (seed[2])
	the number which is multiplied (131) must be a prime number and superior to 256 to avoid collision
 */
void hash(char* password)
{
	uint64_t h = 0;
	for (int j = 0; j < 2; ++j)
	{
		for (int i = 0; i < strlen(password); i++)
		{
			h = 257 * h + password[i];
		}
		seed[j] = h;
	}
}


/*
	-void scramble(FILE* keyFile)
	keyFile : can be null, if present it passes through all the keyfile to scramble the ascii table

	scramble the ascii table assuring that there is no duplicate
	inspired by the Enigma machine; switching letters but without its weekness,
	here a letter can be switched by itself and it is not possible to know how many letters
	have been switched
 */
void scramble(FILE* keyFile){
	char temp = 0;

	for (int i = 0; i < 256; ++i)
	{
		scrambleAsciiTable[i] = i;
	}

	if (keyFile != NULL){
		int size;
		char extractedString[BUFFER_SIZE] = "";
		while((size = fread(extractedString, 1, BUFFER_SIZE, keyFile)) > 0){
			for (int i = 0; i < size; ++i)
			{
				temp = scrambleAsciiTable[i%256];
				scrambleAsciiTable[i%256] = scrambleAsciiTable[(unsigned char)(extractedString[i])];
				scrambleAsciiTable[(unsigned char)(extractedString[i])] = temp;
			}
		}
	} else {
		unsigned char random256;
		for (int i = 0; i < 10 * 256; ++i)
		{
			random256 = generateNumber();
			temp = scrambleAsciiTable[i%256];
			scrambleAsciiTable[i%256] = scrambleAsciiTable[random256];
			scrambleAsciiTable[random256] = temp;
		}
	}
}


/*
	-void unscramble(void)

	this function is here only for optimization
	it inverses the key/value in the scramble ascii table making the backward process instaneous
 */
void unscramble(){
	for (int i = 0; i < 256; ++i)
	{
		unscrambleAsciiTable[(unsigned char) scrambleAsciiTable[i]] = i;
	}
}


/*
	-void codingXOR(char* extractedString, char* keyString, char* xoredString, int bufferLength)
	extractedString : data taken from the source file in a string format
	keyString : a part of the unique key generated by the PRNG in a string format
	xoredString : the result of the xor operation between extractedString and keyString
	bufferLength : the length of the data on which this function is working on

	Apply the mathematical xor function to extractedString and keyString
	if we are coding (isCrypting == 1) then we switche the character from the source file then xor it
	if we are decoding (isCrypting == 0) then we xor the character from the source file then unscramble it
	we can schemate all the coding/decoding xoring process like this :
	coding : 	original:a -> scramble:x -> xored:?
	decoding : 	xored(?) -> unxored(x) -> unscrambled(a)
 */
void codingXOR(char* extractedString, char* keyString, char* xoredString, int bufferLength)
{
	int i;
	for (i = 0; i < bufferLength; ++i)
	{
		xoredString[i] = scrambleAsciiTable[(unsigned char)extractedString[i]] ^ keyString[i];
	}
}


/*
	-void decodingXOR(char* extractedString, char* keyString, char* xoredString, int bufferLength)
	extractedString : data taken from the source file in a string format
	keyString : a part of the unique key generated by the PRNG in a string format
	xoredString : the result of the xor operation between extractedString and keyString
	bufferLength : the length of the data on which this function is working on

	Here only for optimization purpose to limit the amount of conditions
	Apply the mathematical xor function to extractedString and keyString
	if we are coding (isCrypting == 1) then we switche the character from the source file then xor it
	if we are decoding (isCrypting == 0) then we xor the character from the source file then unscramble it
	we can schemate all the coding/decoding xoring process like this :
	coding : 	original(a) -> scramble(x) -> xored(?)
	decoding : 	xored(?) -> unxored(x) -> unscrambled(a)
 */
void decodingXOR(char* extractedString, char* keyString, char* xoredString, int bufferLength)
{
	int i;
	for (i = 0; i < bufferLength; ++i)
	{
		xoredString[i] = unscrambleAsciiTable[(unsigned char)(extractedString[i] ^ keyString[i])];
	}
}


/*
	-void standardXOR(char* extractedString, char* keyString, char* xoredString, int bufferLength)
	extractedString : data taken from the source file in a string format
	keyString : a part of the unique key generated by the PRNG in a string format
	xoredString : the result of the xor operation between extractedString and keyString
	bufferLength : the length of the data on which this function is working on

	Here only for optimization purpose so that there is the small amount
	of condition possible when encrypt or decrypt
	Apply the mathematical xor function to extractedString and keyString
	if we are coding (isCrypting == 1) then we switche the character from the source file then xor it
	if we are decoding (isCrypting == 0) then we xor the character from the source file then unscramble it
	we can schemate all the coding/decoding xoring process like this :
	coding : 	original(a) -> scramble(x) -> xored(?)
	decoding : 	xored(?) -> unxored(x) -> unscrambled(a)
	but here we don't scramble so it is:
	coding : original(a) -> xored(?)
	decoding: xored(?) -> unxored(a)
 */
void standardXOR(char* extractedString, char* keyString, char* xoredString, int bufferLength)
{
	int i;
	for (i = 0; i < bufferLength; ++i)
	{
		xoredString[i] = extractedString[i] ^ keyString[i];
	}
}


/*
	-int fillbuffer(FILE* mainFile, char* extractedString, char* keyString)
	mainFile : pointer to the file given by the user
	extractedString : will contains the data extracted from the source file in a string format
	keyString : will contains a part of the unique key in a string format
	returned value : the size of the data reed

	read a packet of data from the source file
	return the length of the packet which is the buffer size (BUFFER_SIZE)
	it can be less at the final packet (if the file isn't a multiple of the buffer size)

	former version (multiply execution time by 5) :
	int fillBuffer(FILE* mainFile, char* extractedString, char* keyString)
	{
		int i = 0;

		while(!feof(mainFile) && i < BUFFER_SIZE)
		{
			char charBuffer = fgetc(mainFile);
			if (feof(mainFile)) break; //special debug for the last character in text files
			extractedString[i] = charBuffer;
			i++;
		}

		return i;
	}
 */
int fillBuffer(FILE* mainFile, char* extractedString, char* keyString)
{
	int i = 0;

	for (int i = 0; i < BUFFER_SIZE; ++i)
	{
		keyString[i] = generateNumber();
	}

	return fread(extractedString, 1, BUFFER_SIZE, mainFile);
}


/*
	-static inline void loadBar(int x, int n, int r, int w)
	currentIteration : the current iteration of the thing that is proccessed
	maximalIteration : the number which represents 100% of the process
	numberOfSteps : number defining how many times the bar updates
	numberOfSegments : diplayed on w segment

	display a loading bar with current percentage, graphic representation, and time remaining
	which update on every new percent by deleting itself to display the updating bar on top
	inspired by Ross Hemsley's code : https://www.ross.click/2011/02/creating-a-progress-bar-in-c-or-any-other-console-app/

 */
static inline void loadBar(int currentIteration, int maximalIteration, int numberOfSteps, int numberOfSegments)
{
	static char firstCall = 1;
	static double elapsedTime;
	double timeTillEnd;
	static time_t startingTime;
	time_t currentTime;

	if(firstCall){
		startingTime = time(NULL);
		firstCall = 0;
	}

    // numberOfSteps defines the number of times the bar updates.
    if ( currentIteration % (maximalIteration/numberOfSteps + 1) != 0 ) return;

    // Calculate the ratio of complete-to-incomplete.
    float ratio = (float) currentIteration / (float) maximalIteration;
    int loadBarCursorPosition = ratio * numberOfSegments;

    // get the clock now
	currentTime = time(NULL);
	// calculate the remaining time
	elapsedTime = difftime(currentTime, startingTime);
	timeTillEnd = elapsedTime * (1.0/ratio - 1.0);

    // Show the percentage.
    printf(" %3d%% [", (int)(ratio*100));

    // Show the loading bar.
    for (int i = 0; i < loadBarCursorPosition; i++)
       printf("=");

    for (int i = loadBarCursorPosition; i < numberOfSegments; i++)
       printf(" ");

    // go back to the beginning of the line.
    // other way (with ANSI CODE) previous line then erase it : printf("] %.0f\n\033[F\033[J", timeTillEnd);
    printf("] %.0f        \r", timeTillEnd);
    fflush(stdout);
}


/*
	-void code(FILE* mainFile)
	mainFile : pointer to the file given by the user

	Controller for coding the source file
 */
void code (FILE* mainFile)
{
	int mainFileSize = strlen(fileName);
	char codedFileName[mainFileSize+1];
	char extractedString[BUFFER_SIZE] = "";
	char keyString[BUFFER_SIZE] = "";
	char xoredString[BUFFER_SIZE] = "";
	FILE* codedFile;

	sprintf(codedFileName, "%sx%s", pathToMainFile, fileName);
	// opening the output file
	if ((codedFile = fopen(codedFileName, "w+")) == NULL) {
		perror(codedFileName);
		exit(EXIT_FAILURE);
	}

	// starting encryption
	long bufferCount = 0; //keep trace of the task's completion
	printf("starting encryption...\n");
	if (scrambling){
		while(!feof(mainFile))
		{
			int bufferLength = fillBuffer(mainFile, extractedString, keyString);
			codingXOR(extractedString, keyString, xoredString, bufferLength);
			fwrite(xoredString, sizeof(char), bufferLength, codedFile);
			loadBar(++bufferCount, numberOfBuffer, 100, 50);
		}
	} else {
		while(!feof(mainFile))
		{
			int bufferLength = fillBuffer(mainFile, extractedString, keyString);
			standardXOR(extractedString, keyString, xoredString, bufferLength);
			fwrite(xoredString, sizeof(char), bufferLength, codedFile);
			loadBar(++bufferCount, numberOfBuffer, 100, 50);
		}
	}
	// closing the output file
	fclose(codedFile);
	//if the first file was a directory then delete the archive made before crypting
	if (_isADirectory)
	{
		char* tarFile = (char*) malloc (sizeof(char) * (strlen(pathToMainFile) + strlen(fileName) + 1));
		strcpy(tarFile, pathToMainFile);
		strcat(tarFile, fileName);
		remove(tarFile);
		free(tarFile);
	}
}


/*
	-void decode(FILE* mainFile)
	mainFile : pointer to the file given by the user

	controller for decoding the source file
 */
void decode(FILE* mainFile)
{
	int mainFileSize = strlen(fileName);
	char decodedFileName[mainFileSize+1];
	char extractedString[BUFFER_SIZE] = "";
	char keyString[BUFFER_SIZE] = "";
	char xoredString[BUFFER_SIZE] = "";
	FILE* decodedFile;

	// Return the file to a unscramble ascii table
	unscramble();

	// naming the file which will be decrypted
	sprintf(decodedFileName, "x%s", fileName);


	// opening the output file
	strcat(pathToMainFile, decodedFileName);
	if ((decodedFile = fopen(pathToMainFile, "w+")) == NULL) {
		perror(decodedFileName);
		exit(EXIT_FAILURE);
	}

	// starting decryption
	long bufferCount = 0; //keep trace of the task's completion
	printf("starting decryption...\n");
	if(scrambling){
		while(!feof(mainFile))
		{
			int bufferLength = fillBuffer(mainFile, extractedString, keyString);
			decodingXOR(extractedString, keyString, xoredString, bufferLength);
			fwrite(xoredString, sizeof(char), bufferLength, decodedFile);
			loadBar(++bufferCount, numberOfBuffer, 100, 50);
		}
	} else {
		while(!feof(mainFile))
		{
			int bufferLength = fillBuffer(mainFile, extractedString, keyString);
			standardXOR(extractedString, keyString, xoredString, bufferLength);
			fwrite(xoredString, sizeof(char), bufferLength, decodedFile);
			loadBar(++bufferCount, numberOfBuffer, 100, 50);
		}
	}
	// closing the output file
	fclose(decodedFile);
}



/*
	-int isAdirectory(char* path)
	path : string indicated the path of the file/directory
	returned value : 0 or 1

	indicates if the object with this path is a directory or not

*/
int isADirectory(char* path){
	struct stat statStruct;
    int statStatus = stat(path, &statStruct);
    if(-1 == statStatus) {
        if(ENOENT == errno) {
            printf("file's path is not correct, one or several directories and or file are missing\n");
        } else {
            perror("stat");
            exit(1);
        }
    } else {
        if(S_ISDIR(statStruct.st_mode)) {
        	_isADirectory = 1;
            return 1; //it's a directory
        } else {
        	_isADirectory = 0;
            return 0; //it's not a directory
        }
    }
    exit(1);
}



/*
	-int main(int argc, char const* argv[])
	argc : number of arguments passed in the terminal
	argv : pointer to the arguments passed in the terminal
	returned value : 0

 */
int main(int argc, char const *argv[])
{
	FILE* mainFile;
	FILE* keyFile = NULL;

	if ((progName = strrchr(argv[0], '/')) != NULL) {
		++progName;
	} else {
		progName = argv[0];
	}
	if (argc < 2) {
		usage(EXIT_FAILURE);
	} else if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
		usage(EXIT_SUCCESS);
	}
	char *copyOfArgv1 = (char*) malloc(sizeof(char) * strlen(argv[1]));
	strcpy(copyOfArgv1, argv[1]);

	//outside his scope because we need to free it at the end
	char* tarName;
	char* dirName;
	if (isADirectory(copyOfArgv1)){
		char command[1008];
		//we don't need that anymore
		free(copyOfArgv1);
		printf("regrouping the folder in one file...");
		// get the name of the folder in a string and get the path
		if ((fileName = strrchr(argv[1], '/')) != NULL) {
			//if the '/' is the last character in the string, delete it and get the fileName again
			if (strlen(fileName) == 1){
				dirName = (char*) malloc( sizeof(char) * strlen(argv[1]) + 5);
				strcpy(dirName, argv[1]);
				*(dirName+(fileName-argv[1])) = '\0';
				if ((fileName = strrchr(dirName, '/')) != NULL){
					++fileName;
					strncpy(pathToMainFile, dirName, fileName - dirName);
					pathToMainFile[fileName - dirName] = '\0';
				}
				else{
					fileName = dirName;
				}
			}
			else {
				++fileName;
				strncpy(pathToMainFile, argv[1], fileName - argv[1]);
				pathToMainFile[fileName - argv[1]] = '\0';
			}
		}
		else {
			fileName = argv[1];
			printf("\n%s\n", fileName);
		}
		// get the full path of the tarFile in a dynamic variable tarName
		tarName = (char*) malloc(sizeof(char) * strlen(fileName) + 5);
		sprintf (tarName, "%s.tar", fileName);
		sprintf (command, "cd %s && tar -cf %s %s &>/dev/null", pathToMainFile, tarName, fileName);
		// make the archive of the folder with tar
		system(command);
		printf("done\n");
		fileName = tarName;

		// trying to open the new archive
		char pathPlusName[strlen(pathToMainFile)+strlen(fileName)];
		sprintf(pathPlusName, "%s%s", pathToMainFile, fileName);
		if ((mainFile = fopen(pathPlusName, "r")) == NULL) {
			perror(pathPlusName);
			return EXIT_FAILURE;
		}
	}
	else{
		if ((fileName = strrchr(argv[1], '/')) != NULL) {
			++fileName;
			strncpy(pathToMainFile, argv[1], fileName - argv[1]);		
		} else {
			fileName = argv[1];
		}
		if ((mainFile = fopen(argv[1], "r")) == NULL) {
			perror(argv[1]);
			return EXIT_FAILURE;
		}
	}

	if (argc >= 3)
	{
		if (strcmp(argv[2], "-s") == 0 || strcmp(argv[2], "--standard") == 0){
		scrambling = 0;
		} else if ((keyFile = fopen(argv[2], "r")) == NULL) {
			perror(argv[1]);
			return EXIT_FAILURE;
		}
	}

	fseek(mainFile, 0, SEEK_END);
	long mainFileSize = ftell(mainFile);
	rewind(mainFile);
	numberOfBuffer = ceilRound((float)mainFileSize / (float)(BUFFER_SIZE));
	if (numberOfBuffer < 1)
	{
		numberOfBuffer = 1;
	}

	char procedureResponse[10]; 
	printf("Crypt(C) or Decrypt(D):");
	fgets (procedureResponse, 9, stdin);
	if (procedureResponse[0] == 'C' || procedureResponse[0] == 'c') {
		isCrypting = 1;
	}
	else {
		isCrypting = 0;
	}

	char passPhrase[16384];
	printf("Password:");
	fgets (passPhrase, 16383, stdin);
	printf("\033[F\033[J");
	hash(passPhrase);
	scramble(keyFile);

	if (isCrypting){
		code(mainFile);
	}
	else{
		decode(mainFile);
	}

	printf("done                                                                  \n");
	fclose(mainFile);

	//we can free (last use in code/decode)
	free(tarName);
	free(dirName);

	return 0;
}
