#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define BYTES 256

//****************************** Min Heap ******************************
struct _MinHeapNode;
typedef struct _MinHeapNode MinHeapNode;
typedef struct _MinHeapNode* Position;

struct _MinHeapNode
{
	char slovo;
	unsigned int freq;

	// Used only for building a huffman tree
	// Heap part is implemented in array by MinHeap struct
	Position left;
	Position right;
};

struct _MinHeap;
typedef struct _MinHeap MinHeap;

struct _MinHeap
{
	int lastIndex;
	int size;

	Position array;
};

Position createNode(char slovo, unsigned int freq);

MinHeap* createMinHeap(unsigned int *freqArr, int size);

int PrintArr(MinHeap *heap);

int Parent(int size, int index);

int LeftChild(int size, int index);

int RightChild(int size, int index);

int IsEmptyHeap(int lastIndex);

int MinHeapify(MinHeap *heap, int index);

int BuildMinHeap(MinHeap *heap);

MinHeap* CreateAndBuildHeap(unsigned int *freqArr, int size);

Position GetMin(MinHeap *heap);

// Removes min node from root then sorts again
MinHeapNode ExtractMin(MinHeap *heap);

int Insert(MinHeap *heap, char slovo, unsigned int freq);

int Insert(MinHeap *heap, Position node);

unsigned int TreeMaxLevel(Position node);

//****************************** END ******************************


//****************************** Prep work for min heap ******************************

int countLetterFreq(const char *fileName, unsigned int *resultArr);

// Ova funkcija ce nam vratiti kao rezultat broj koji nam govori
// koliko se znakova od ASCII tablice koristi
int usedChars(unsigned int *freqArr);

//****************************** END ******************************

// Returns root node
Position BuildHuffmanTree(MinHeap *heap);

int PrintHuffmanCode(Position root, int codes[], int treeLevel);

int ReadHuffmanCodeIntoArray(Position node, int codes[], int treeLevel, char *codesArr[]);

int EncodeToFile(const char *filename, const char *compressedFilename, Position huffmanTree, unsigned int * freq, char *stringCodes[], unsigned int levelSize);
int DecodeToFile(const char *filename, const char *compressedFilename);

char IsLetter(Position node, int codes[], int currentLen);

int main(int numOfArgs, char **args)
{
	const char *fileName = "test.txt";

	// Ovaj niz ce raditi na principu da provjerava sva slova iz ASCII tablice i svaki
	// put kada postoji to slovo mu povecaje vrijednost za 1
	unsigned int frequency[BYTES];
	char *stringCodes[BYTES];

	int i;
	unsigned int levelSize;
	// Koristi se za funkciju ReadHuffmanCodes
	int *codes;

	MinHeap *minHeap = NULL;
	Position huffmanTree = NULL;

	// Koristi se za heap array, da bi se znala kolika je njegova velicina
	int size = 0;

	memset(frequency, 0, BYTES * sizeof(int));

	countLetterFreq(fileName, frequency);
	size = usedChars(frequency);

	// Heap creation
	minHeap = CreateAndBuildHeap(frequency, size);
	//PrintArr(minHeap);

	huffmanTree = BuildHuffmanTree(minHeap);
	levelSize = TreeMaxLevel(huffmanTree);

	codes = (int*)malloc(levelSize * sizeof(int));
	assert(codes != NULL);

	//ReadHuffmanCode(huffmanTree, codes, 0);

	// Allocating memory for string codes that will be used to encode file
	for (i = 0; i < BYTES; ++i)
	{
		stringCodes[i] = (char *)(malloc((levelSize + 1) * sizeof(char)));
		assert(stringCodes[i] != NULL);
		memcpy(stringCodes[i], "aa", (levelSize + 1) * sizeof(char));
	}

	ReadHuffmanCodeIntoArray(huffmanTree, codes, 0, stringCodes);
	EncodeToFile(fileName, "test.bin", huffmanTree, frequency, stringCodes, levelSize);

	DecodeToFile("test.bin", "test1.txt");

	getchar();
	getchar();
	return 0;
}

int countLetterFreq(const char * fileName, unsigned int * resultArr)
{
	char letter;
	unsigned int index = 0;
	FILE *fp = NULL;
	fp = fopen(fileName, "r");
	if (fp == NULL)
	{
		printf("ERROR::File not found!\n");
		return -1;
	}

	while (!feof(fp))
	{
		fscanf(fp, "%c", &letter);
		index = (int)letter;
		// Za svako slovo pronalazi njegov index i inkrementira ga za jedan
		resultArr[index]++;
	}

	fclose(fp);
	return 0;
}

int usedChars(unsigned int * freqArr)
{
	int i;
	int count = 0;
	if (freqArr == NULL)
		exit(-1);

	for (i = 0; i < BYTES; ++i)
	{
		if (freqArr[i] > 0)
			count++;
	}

	return count;
}

Position createNode(char slovo, unsigned int freq)
{
	MinHeapNode *newNode = (MinHeapNode*)malloc(sizeof(MinHeapNode));
	if (newNode == NULL)
	{
		printf("ERROR::createNode()::Could not allocate enough memory!\n");
		return NULL;
	}

	newNode->slovo = slovo;
	newNode->freq = freq;

	newNode->left = NULL;
	newNode->right = NULL;
	return newNode;
}

// Creates heap array out of used frequencies
MinHeap* createMinHeap(unsigned int * freqArr, int size)
{
	MinHeap *heap;
	int i, count;
	count = 0;
	heap = (MinHeap*)malloc(sizeof(MinHeap));
	if (heap == NULL)
	{
		printf(("ERROR::Could not allocate enough memory!\n"));
		return NULL;
	}

	heap->array = (MinHeapNode*)malloc(size * sizeof(MinHeapNode));
	heap->size = size;
	heap->lastIndex = size - 1;
	if (heap->array == NULL)
	{
		printf(("ERROR::Could not allocate enough memory!\n"));
		return NULL;
	}

	for (i = 0; i < BYTES; ++i)
	{
		unsigned int a = freqArr[i];
		if (freqArr[i] > 0)
			heap->array[count++] = *createNode((char)i, freqArr[i]);

		// To znaci da smo ucitali sva slova koja se koriste
		// pa prekidamo izvodenje petlje
		if (count == size) break;
	}

	return heap;
}

int PrintArr(MinHeap *heap)
{
	int i;
	MinHeapNode *arr = heap->array;
	int size = heap->size;

	if (heap == NULL) return -1;

	for (i = 0; i < size; ++i)
	{
		if (arr[i].freq == INT_MAX) break;
		printf("Slovo: %c Freq: %u\n", arr[i].slovo, arr[i].freq);
	}

	return 0;
}

int Parent(int size, int index)
{
	int parentIndex = (int)ceil(index / 2.0 - 1);
	if (index >= size) exit(-1);
	if (parentIndex < 0) exit(-1);

	return parentIndex;
}

int LeftChild(int size, int index)
{
	int leftChildIndex = index * 2 + 1;
	return leftChildIndex;
}

int RightChild(int size, int index)
{
	int rightChildIndex = index * 2 + 2;
	return rightChildIndex;
}

int IsEmptyHeap(int lastIndex)
{
	return (lastIndex < 0) ? 1 : 0;
}

int MinHeapify(MinHeap *heap, int index)
{
	MinHeapNode temp;
	Position arr = heap->array;
	int size = heap->size;

	int leftChildIndex, rightChildIndex;
	leftChildIndex = LeftChild(size, index);
	rightChildIndex = RightChild(size, index);

	if (leftChildIndex >= size) return 0;
	if (rightChildIndex > size) return 0;

	if (arr[index].freq < arr[leftChildIndex].freq && arr[index].freq < arr[rightChildIndex].freq) return -1;

	if (arr[leftChildIndex].freq < arr[rightChildIndex].freq)
	{
		temp = arr[leftChildIndex];
		arr[leftChildIndex] = arr[index];
		arr[index] = temp;

		MinHeapify(heap, leftChildIndex);
	}
	else
	{
		temp = arr[rightChildIndex];
		arr[rightChildIndex] = arr[index];
		arr[index] = temp;

		MinHeapify(heap, rightChildIndex);
	}

	return 0;
}

int BuildMinHeap(MinHeap *heap)
{
	int size = heap->size;
	Position arr = heap->array;

	int startIndex = size / 2 - 1;
	int i;

	for (i = startIndex; i >= 0; --i)
	{
		MinHeapify(heap, i);
	}

	return 0;
}

MinHeap* CreateAndBuildHeap(unsigned int * freqArr, int size)
{
	MinHeap *heap = createMinHeap(freqArr, size);
	if (heap == NULL) return NULL;
	BuildMinHeap(heap);

	return heap;
}

MinHeapNode* GetMin(MinHeap *heap)
{
	if (heap->lastIndex < 1)
	{
		printf("Heap is empty!");
		return NULL;
	}

	return &(heap->array[0]);
}

MinHeapNode ExtractMin(MinHeap *heap)
{
	MinHeapNode min;
	MinHeapNode temp;
	Position arr = heap->array;
	int *lastIndex = &(heap->lastIndex);

	if (*lastIndex < 0)
	{
		printf("Heap is empty!");
		assert(*lastIndex > 0);
	}

	min = heap->array[0];

	arr[0] = arr[*lastIndex];
	arr[*lastIndex].freq = INT_MAX;
	(*lastIndex)--;

	// Only 2 nodes left
	if (*lastIndex == 1)
	{
		temp = arr[*lastIndex];
		arr[*lastIndex] = arr[0];
		arr[0] = temp;
		return min;
	}

	MinHeapify(heap, 0);

	return min;
}

int Insert(MinHeap *heap, char slovo, unsigned int freq)
{
	MinHeapNode *newNode, temp;
	int *lastIndex = &heap->lastIndex;
	int size = heap->size;
	Position arr = heap->array;
	int i;
	if (*lastIndex == size - 1)
	{
		printf("Heap capacity full!\n");
		return -1;
	}

	(*lastIndex)++;
	i = *lastIndex;
	newNode = createNode(slovo, freq);
	arr[*lastIndex] = *newNode;

	while (i != 0 && arr[Parent(*lastIndex + 1, i)].freq > arr[i].freq)
	{
		temp = arr[Parent(*lastIndex + 1, i)];
		arr[Parent(*lastIndex + 1, i)] = arr[i];
		arr[i] = temp;

		i = Parent(*lastIndex + 1, i);
	}
	return 0;
}

int Insert(MinHeap *heap, Position node)
{
	MinHeapNode temp;

	int *lastIndex = &heap->lastIndex;
	int size = heap->size;

	Position arr = heap->array;
	int i;

	if (*lastIndex == size - 1)
	{
		printf("Heap capacity full!\n");
		return -1;
	}

	(*lastIndex)++;
	i = *lastIndex;
	arr[*lastIndex] = *node;

	while (i != 0 && arr[Parent(*lastIndex + 1, i)].freq > arr[i].freq)
	{
		temp = arr[Parent(*lastIndex + 1, i)];
		arr[Parent(*lastIndex + 1, i)] = arr[i];
		arr[i] = temp;

		i = Parent(*lastIndex + 1, i);
	}
	return 0;
}

unsigned int TreeMaxLevel(Position node)
{
	if (!node) return 0;
	else
	{
		int lHeight = TreeMaxLevel(node->left);
		int rHeight = TreeMaxLevel(node->right);

		if (lHeight > rHeight) return lHeight + 1;
		else return rHeight + 1;
	}
}

Position BuildHuffmanTree(MinHeap *heap)
{
	Position result, left, right, rootNode;
	MinHeapNode min1, min2;
	Position arr = heap->array;
	int *lastIndex = &heap->lastIndex;

	while (*lastIndex > 0)
	{
		min1 = ExtractMin(heap);
		min2 = ExtractMin(heap);

		result = createNode(NULL, min1.freq + min2.freq);

		left = createNode(min1.slovo, min1.freq);
		right = createNode(min2.slovo, min2.freq);

		if (!left || !right)
		{
			printf("ERROR::Cannot allocate enough memory!\n");
			return NULL;
		}

		left->left = min1.left;
		left->right = min1.right;
		right->left = min2.left;
		right->right = min2.right;

		result->left = left;
		result->right = right;

		Insert(heap, result);
	}

	rootNode = createNode(NULL, 0);

	// koristi se kao temp sada
	min1 = ExtractMin(heap);
	rootNode->slovo = min1.slovo;
	rootNode->freq = min1.freq;
	rootNode->left = min1.left;
	rootNode->right = min1.right;

	return rootNode;
}

int PrintHuffmanCode(Position node, int codes[], int treeLevel)
{
	int i;
	if (node == NULL) return 0;

	if (node->left != NULL)
	{
		codes[treeLevel] = 0;
		PrintHuffmanCode(node->left, codes, treeLevel + 1);
	}

	if (node->right != NULL)
	{
		codes[treeLevel] = 1;
		PrintHuffmanCode(node->right, codes, treeLevel + 1);
	}

	// Dosli smo do slova, jer je ovo list
	if (node->left == NULL && node->right == NULL)
	{
		printf("Slovo: %c\t\tCode: ", node->slovo);
		for (i = 0; i < treeLevel; ++i)
		{
			printf("%d", codes[i]);
		}
		printf("\n");
	}

	return 0;
}

int ReadHuffmanCodeIntoArray(Position node, int codes[], int treeLevel, char *codesArr[])
{
	int i;
	if (node == NULL) return -1;

	if (node->left != NULL)
	{
		codes[treeLevel] = 0;
		ReadHuffmanCodeIntoArray(node->left, codes, treeLevel + 1, codesArr);
	}

	if (node->right != NULL)
	{
		codes[treeLevel] = 1;
		ReadHuffmanCodeIntoArray(node->right, codes, treeLevel + 1, codesArr);
	}

	// Dosli smo do slova, jer je ovo list
	if (node->left == NULL && node->right == NULL)
		for (i = 0; i < treeLevel; ++i)
			sprintf(&codesArr[node->slovo][i], "%d", codes[i]);

	return 0;
}

int EncodeToFile(const char *filename, const char *compressedFilename, Position huffmanTree, unsigned int * freq, char *stringCodes[], unsigned int levelSize)
{
	int i;
	// Slovo koje se zapisuje u file
	char letter;
	char code = 0;
	char *buff = NULL;
	// Koristi se za pracenje da li je stavljeno 8 bita
	// u 'code' varijablu
	short int count = 0;

	// Varijabla za pracenje koliko je byteova procitano iz file-a
	// da bi se na kraju moglo ispisati koliko je ustedeno
	unsigned int bytesInSource = 0;
	unsigned int bytesInCompress = 0;
	float savedSpaceResult = 0.0f;

	// Frekvencija koja se zapisuje u file
	unsigned int f = 0;
	FILE *compressedFile = NULL;
	FILE *fileToRead = NULL;

	buff = (char *)malloc((levelSize + 1) * sizeof(char));
	if (buff == NULL)
	{
		printf("ERROR::EncodeToFile::Could not allocate enough memory!\n");
		return -1;
	}
	compressedFile = fopen(compressedFilename, "wb");

	// Izrada tablice slova i frekvencije.
	// 1B za slovo i 4B za frekvenciju
	// Koristi se kada se file bude dekompresirao da se nanovo izgradi Huffmanovo stablo
	for (i = 0; i < BYTES; ++i)
	{
		if (freq[i] == 0) continue;

		letter = i;
		f = freq[i];
		fwrite(&letter, sizeof(char), 1, compressedFile);
		fwrite(&f, sizeof(unsigned int), 1, compressedFile);

		bytesInCompress += 5;
	}

	// Zapis tablice je gotov. Dodaje se 8B punih jedinica da bi se znalo da pocinju enkodirani znakovi
	f = UINT_MAX;
	fwrite(&f, sizeof(unsigned int), 1, compressedFile);
	fwrite(&f, sizeof(unsigned int), 1, compressedFile);

	bytesInCompress += 8;

	// Otvaranje file-a kojeg se treba kompresirati
	// Citanje tog file i istovremeni zapis u drugi file
	// kada se letter napuni sa 8 bita

	fileToRead = fopen(filename, "r");
	assert(fileToRead != NULL);

	while (!feof(fileToRead))
	{
		fscanf(fileToRead, "%c", &letter);
		// Kopiraj huffman kod iz stringCodes niza u buffer
		strcpy(buff, stringCodes[(int)letter]);
		bytesInSource += 1;
		i = 0;
		while (buff[i] != '\0')
		{
			if (count < 8)
			{
				code = code << 1;
				code |= (int)buff[i] - 48;
				count++;
				++i;
			}
			else
			{
				fwrite(&code, sizeof(char), 1, compressedFile);
				bytesInCompress += 1;
				count = 0;
				code = 0;
			}
		}
	}
	fwrite(&code, sizeof(char), 1, compressedFile);
	bytesInCompress += 1;

	fclose(fileToRead);
	fileToRead = NULL;

	fclose(compressedFile);
	compressedFile = NULL;

	printf("Bytes in source file: %u\nBytes in compressed file: %u\n", bytesInSource, bytesInCompress);
	savedSpaceResult = 1.0 - (float)bytesInCompress / bytesInSource;
	printf("Saved: %f %%\n", savedSpaceResult * 100);
	return 1;
}

int DecodeToFile(const char * filename, const char * compressedFilename)
{
	FILE *compressedFile = NULL, *uncompressedFile = NULL;

	MinHeap *minHeap = NULL;
	Position huffmanTree = NULL;

	unsigned int levelSize;

	// Prvo je potrebno izvuci tablicu iz file-a
	unsigned int frequency[BYTES];

	// Flag koji kada dode do 0 znaci da smo procitali tablicu i prosli 8B jedinica
	int endOfTable = 8;

	int size = 0;
	unsigned int i;
	char letter;
	int codeCount = 7;
	char code = 0;
	int *huffmanCodesTemp = NULL;
	int *codes = NULL;

	memset(frequency, 0, BYTES * sizeof(int));

	compressedFile = fopen(filename, "rb");
	if (compressedFile == NULL) return -1;

	while (!feof(compressedFile))
	{
		fread(&letter, sizeof(char), 1, compressedFile);
		i = letter;
		if (i > 255)
		{
			--endOfTable;
			// Znaci da smo dosli do 8B punih jedinica
			if (endOfTable == 0)
				break;
			continue;
		}
		fread(&frequency[i], sizeof(unsigned int), 1, compressedFile);
	}

	// Sagradi heap i huffmanovo stablo
	size = usedChars(frequency);

	// Heap creation
	minHeap = CreateAndBuildHeap(frequency, size);

	//PrintArr(minHeap);
	huffmanTree = BuildHuffmanTree(minHeap);
	levelSize = TreeMaxLevel(huffmanTree);

	codes = (int*)malloc(levelSize * sizeof(int));
	assert(codes != NULL);

	//PrintHuffmanCode(huffmanTree, codes, 0);

	// U ovaj niz cemo dodavati trenutni kod za neki broj
	huffmanCodesTemp = (int *)malloc((levelSize + 1) * sizeof(int));
	if (huffmanCodesTemp == NULL) return -1;
	memset(huffmanCodesTemp, 0, (levelSize + 1) * sizeof(int));
	// Ucitamo byte i pratimo stablo po byteovima
	uncompressedFile = fopen(compressedFilename, "wb");
	fread(&code, sizeof(char), 1, compressedFile);
	while (!feof(compressedFile))
	{
		i = 0;
		memset(huffmanCodesTemp, 0, (levelSize + 1) * sizeof(int));
		while ((letter = IsLetter(huffmanTree, huffmanCodesTemp, i)) == -1)
		{
			huffmanCodesTemp[i++] = (code & 128) ? 1 : 0;
			code = code << 1;
			codeCount--;
			if (codeCount < 0)
			{
				fread(&code, sizeof(char), 1, compressedFile);
				codeCount = 7;
			}
		}
		fwrite(&letter, sizeof(char), 1, uncompressedFile);

	}

	fclose(uncompressedFile);
	uncompressedFile = NULL;
	fclose(compressedFile);
	compressedFile = NULL;

	return 0;
}

char IsLetter(Position node, int codes[], int currentLen)
{
	Position it = node;
	int i = 0;
	while (i <= currentLen)
	{
		if (it->left == NULL && it->right == NULL)
			return it->slovo;

		if (codes[i] == 0)
			it = it->left;

		else if (codes[i] == 1)
			it = it->right;

		++i;
	}
	return -1;
}
