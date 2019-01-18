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

typedef struct
{
	FILE *compressedFile, *uncompressedFile;

	// minHeap (priority queue) se koristi kako bismo mogli brojevima
	// dodijeliti njihove binarne vrijednosti, na racun njihovih frekventnih pojava
	// u tekstualnoj datoteci.
	MinHeap *minHeap;

	// Pozvati BuildHuffmanTree i proslijediti prethodno napravljeni minHeap
	// za kreaciju Huffmanovog stabla
	MinHeapNode *huffmanTree;

	// Ovaj niz ce raditi na principu da provjerava sva slova iz ASCII tablice i svaki
	// put kada postoji to slovo mu povecaje vrijednost za 1
	unsigned int frequencies[BYTES];


	// Ova varijabla se koristi da bismo znali koja je maksimalna duljina stabla.
	// Sa tim saznanjem znamo koja je najveca moguca duljina bitova koju neki znak
	// moze poprimiti.
	unsigned int levelSize;

	// Koristi se za heap array, da bi se znala kolika je njegova velicina
	// poziva se funkcija usedChars(freq) da bi se dobila vrijednost koja nam
	// govori koliko ASCII znakova se koristi u datoteci
	int uniqueChars;

} CompressionData;

// Funkcija koja se koristi kod enkodiranja
int CompressionDataInitEncode(CompressionData *data, const char *uncompressedFile);

int CompressionDataInitValsZero(CompressionData *data);

int CompressionDataDestruct(CompressionData *data);

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

int InsertNode(MinHeap *heap, Position node);

unsigned int TreeMaxLevel(Position node);

//****************************** END ******************************

//****************************** Prep work for min heap ******************************

int countLetterFreq(const char *fileName, unsigned int *resultArr);

// Ova funkcija ce nam vratiti kao rezultat broj koji nam govori
// koliko se znakova od ASCII tablice koristi
int usedChars(unsigned int *freqArr);

void* Create(size_t bytes, short int isZero);
int Free(void *ptr);
void* CreateArray(int n, size_t bytes, short int writeZeros);
int FreeArray(void **ptr, int n);
int FreeTree(Position node);

//****************************** END ******************************

// Returns root node
Position BuildHuffmanTree(MinHeap *heap);

int PrintHuffmanCode(Position root, int codes[], int treeLevel);

int ReadHuffmanCodeIntoArray(Position node, int codes[], int treeLevel, char *codesArr[]);

int EncodeToFile(const char *filename, const char *compressedFilename, short int showLog);

int DecodeToFile(const char *compressedFilename, const char *uncompressedFilename, short int showLog);

char IsLetter(Position node, int codes[], int currentLen);

int Izbornik();

int main()
{
	Izbornik();
	return 0;
}

int countLetterFreq(const char * fileName, unsigned int * resultArr)
{
	char letter;
	unsigned int index = 0;
	FILE *fp = NULL;

	fp = fopen(fileName, "r");
	if (fp == NULL) printf("ERROR::File not found!\n");
	assert(fp != NULL);

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

void* Create(size_t bytes, short int writeZeros)
{
	void *ptr = NULL;
	if (writeZeros)
		ptr = calloc(1, bytes);
	else
		ptr = malloc(bytes);

	if (ptr == NULL)
		printf("ERROR::Create()::Could not allocate enough memory!\n");

	assert(ptr != NULL);
	return ptr;
}

int Free(void * ptr)
{
	if (ptr == NULL) return -1;

	free(ptr);
	ptr = NULL;

	return 0;
}

void * CreateArray(int n, size_t bytes, short int writeZeros)
{
	void *arr = NULL;
	if (writeZeros)
		arr = calloc(n, bytes);
	else
		arr = malloc(n * bytes);

	if (arr == NULL)
		printf("ERROR::CreateArray()::Could not allocate enough memory!\n");

	assert(arr != NULL);
	return arr;
}

int FreeArray(void ** ptr, int n)
{
	if (ptr == NULL) return -1;

	int i;
	for (i = 0; i < n; ++i)
		Free(ptr[i]);

	return 0;
}

int FreeTree(Position node)
{
	if (node == NULL) return 0;

	FreeTree(node->left);
	FreeTree(node->right);

	Free(node);
	return 0;
}

int CompressionDataInitEncode(CompressionData * data, const char *uncompressedFile)
{
	CompressionDataInitValsZero(data);

	countLetterFreq(uncompressedFile, data->frequencies);
	data->uniqueChars = usedChars(data->frequencies);

	data->minHeap = CreateAndBuildHeap(data->frequencies, data->uniqueChars);
	assert(data->minHeap != NULL);

	data->huffmanTree = BuildHuffmanTree(data->minHeap);
	assert(data->huffmanTree != NULL);

	data->levelSize = TreeMaxLevel(data->huffmanTree);
	return 0;
}

int CompressionDataInitValsZero(CompressionData * data)
{
	data->compressedFile = NULL;
	data->uncompressedFile = NULL;
	data->minHeap = NULL;
	data->huffmanTree = NULL;
	data->levelSize = 0;
	data->uniqueChars = 0;

	memset(data->frequencies, 0, BYTES * sizeof(unsigned int));

	return 0;
}

int CompressionDataDestruct(CompressionData * data)
{
	//Free(codes);
	//FreeArray((void **)stringCodes, BYTES);
	Free(data->minHeap->array);
	Free(data->minHeap);
	FreeTree(data->huffmanTree);

	fclose(data->compressedFile);
	data->compressedFile = NULL;
	fclose(data->uncompressedFile);
	data->uncompressedFile = NULL;

	return 0;
}

Position createNode(char slovo, unsigned int freq)
{
	MinHeapNode *newNode = (MinHeapNode *)Create(sizeof(MinHeapNode), 0);

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
	heap = (MinHeap*)Create(sizeof(MinHeap), 0);

	//heap->array = (MinHeapNode*)malloc(size * sizeof(MinHeapNode));
	heap->array = (MinHeapNode*)CreateArray(size, sizeof(MinHeapNode), 0);
	heap->size = size;
	heap->lastIndex = size - 1;

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

int InsertNode(MinHeap *heap, Position node)
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

		InsertNode(heap, result);
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

int EncodeToFile(const char *filename, const char *compressedFilename, short int showLog)
{
	CompressionData compData;

	int i;
	// Slovo koje se zapisuje u file
	char letter;
	char code = 0;
	char *buff = NULL;
	// Koristi se za pracenje da li je stavljeno 8 bita
	// u 'code' varijablu
	short int count = 0;
	char *stringCodes[BYTES];

	// Koristi se za funkciju ReadHuffmanCodes
	int *codes;


	// Varijabla za pracenje koliko je byteova procitano iz file-a
	// da bi se na kraju moglo ispisati koliko je ustedeno
	unsigned int bytesInSource = 0;
	unsigned int bytesInCompress = 0;
	float savedSpaceResult = 0.0f;

	// Frekvencija koja se zapisuje u file
	unsigned int f = 0;

	CompressionDataInitEncode(&compData, filename);

	buff = (char *)CreateArray(compData.levelSize + 1, sizeof(char), 0);
	compData.compressedFile = fopen(compressedFilename, "wb");
	assert(compData.compressedFile != NULL);

	codes = (int *)CreateArray(compData.levelSize, sizeof(int), 0);

	// Allocating memory for string codes that will be used to encode file
	for (i = 0; i < BYTES; ++i)
	{
		stringCodes[i] = (char *)CreateArray(compData.levelSize + 1, sizeof(char), 0);
		memcpy(stringCodes[i], "aa", (compData.levelSize + 1) * sizeof(char));
	}

	ReadHuffmanCodeIntoArray(compData.huffmanTree, codes, 0, stringCodes);

	if (showLog)
	{
		printf("Encode:\n\n");
		PrintHuffmanCode(compData.huffmanTree, codes, 0);
		printf("\n\n");
	}


	// Izrada tablice slova i frekvencije.
	// 1B za slovo i 4B za frekvenciju
	// Koristi se kada se file bude dekompresirao da se nanovo izgradi Huffmanovo stablo
	for (i = 0; i < BYTES; ++i)
	{
		if (compData.frequencies[i] == 0) continue;

		letter = i;
		f = compData.frequencies[i];
		fwrite(&letter, sizeof(char), 1, compData.compressedFile);
		fwrite(&f, sizeof(unsigned int), 1, compData.compressedFile);

		bytesInCompress += 5;
	}

	// Zapis tablice je gotov. Dodaje se 8B punih jedinica da bi se znalo da pocinju enkodirani znakovi
	f = UINT_MAX;
	fwrite(&f, sizeof(unsigned int), 1, compData.compressedFile);
	fwrite(&f, sizeof(unsigned int), 1, compData.compressedFile);

	bytesInCompress += 8;

	// Otvaranje file-a kojeg se treba kompresirati
	// Citanje tog file i istovremeni zapis u drugi file
	// kada se letter napuni sa 8 bita

	compData.uncompressedFile = fopen(filename, "rb");
	assert(compData.uncompressedFile != NULL);

	while (!feof(compData.uncompressedFile))
	{
		fscanf(compData.uncompressedFile, "%c", &letter);
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
				fwrite(&code, sizeof(char), 1, compData.compressedFile);
				bytesInCompress += 1;
				count = 0;
				code = 0;
			}
		}
	}
	fwrite(&code, sizeof(char), 1, compData.compressedFile);
	bytesInCompress += 1;

	FreeArray((void **)stringCodes, BYTES);

	printf("Bytes in source file: %u\nBytes in compressed file: %u\n", bytesInSource, bytesInCompress);
	savedSpaceResult = 1.0f - (float)bytesInCompress / bytesInSource;
	printf("Saved: %f %%\n", savedSpaceResult * 100);

	CompressionDataDestruct(&compData);

	return 1;
}

int DecodeToFile(const char *compressedFilename, const char *uncompressedFilename, short int showLog)
{
	CompressionData compData;

	// Flag koji kada dode do 0 znaci da smo procitali tablicu i prosli 8B jedinica
	int endOfTable = 8;

	int size = 0;
	unsigned int i;
	char letter;
	int codeCount = 7;
	char code = 0;
	int *huffmanCodesTemp = NULL;
	int *codes = NULL;

	CompressionDataInitValsZero(&compData);
	compData.compressedFile = fopen(compressedFilename, "rb");
	if (compData.compressedFile == NULL) printf("ERROR::DecodeToFile()::Couldn't find specified file path!\n");
	assert(compData.compressedFile != NULL);

	// Iscitavanje tablice frekvencija
	while (!feof(compData.compressedFile))
	{
		fread(&letter, sizeof(char), 1, compData.compressedFile);
		i = letter;
		if (i > 255)
		{
			--endOfTable;
			// Znaci da smo dosli do 8B punih jedinica
			if (endOfTable == 0)
				break;
			continue;
		}
		fread(&compData.frequencies[i], sizeof(unsigned int), 1, compData.compressedFile);
	}

	compData.uniqueChars = usedChars(compData.frequencies);

	compData.minHeap = CreateAndBuildHeap(compData.frequencies, compData.uniqueChars);
	assert(compData.minHeap != NULL);

	compData.huffmanTree = BuildHuffmanTree(compData.minHeap);
	assert(compData.huffmanTree != NULL);

	compData.levelSize = TreeMaxLevel(compData.huffmanTree);

	codes = (int *)CreateArray(compData.levelSize, sizeof(int), 0);

	if (showLog)
	{
		printf("Decode:\n\n");
		PrintHuffmanCode(compData.huffmanTree, codes, 0);
		printf("\n\n");
	}

	// U ovaj niz cemo dodavati trenutni kod za neki broj
	huffmanCodesTemp = (int *)CreateArray(compData.levelSize + 1, sizeof(int), 1);

	// Ucitamo byte i pratimo stablo po byteovima
	compData.uncompressedFile = fopen(uncompressedFilename, "wb");
	fread(&code, sizeof(char), 1, compData.compressedFile);

	while (!feof(compData.compressedFile))
	{
		i = 0;
		memset(huffmanCodesTemp, 0, (compData.levelSize + 1) * sizeof(int));
		while ((letter = IsLetter(compData.huffmanTree, huffmanCodesTemp, i)) == -1)
		{
			huffmanCodesTemp[i++] = (code & 128) ? 1 : 0;
			code = code << 1;
			codeCount--;
			if (codeCount < 0)
			{
				fread(&code, sizeof(char), 1, compData.compressedFile);
				codeCount = 7;
			}
		}
		fwrite(&letter, sizeof(char), 1, compData.uncompressedFile);

	}

	CompressionDataDestruct(&compData);
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

int Izbornik()
{
	int ans = 0;
	char buff1[BYTES];
	char buff2[BYTES];
	while (ans < 1 || ans > 5)
	{
		printf("\t\t\tDobrodosli u Huffman kompresijski program\n\n");
		printf("Molim vas odaberite jednu od ispisanih opcija\n");
		printf("\n");
		printf("\t1. Zapakiraj tekstualni file\n");
		printf("\t2. Zapakiraj tekstualni file i ispisi Huffman kodove u konzoli\n");
		printf("\t3. Odpakiraj file\n");
		printf("\t4. Odpakiraj file i ispisi Huffman kodove u konzoli\n");
		printf("\t5. Izlaz iz programa");
		printf("\n\n");
		printf("Vas izbor: ");

		scanf("%d", &ans);
	}
	if (ans == 5) return 1;

	switch (ans)
	{
	case 1:
		printf("Unesite ime file-a (dodajte ekstenziju takoder):");
		scanf("%s", buff1);
		printf("Unesite novo ime za zapakirani file + ekstenzija: ");
		scanf("%s", buff2);

		EncodeToFile(buff1, buff2, 0);
		printf("\nEncoded!\n");

		break;
	case 2:
		printf("Unesite ime file-a (dodajte ekstenziju takoder):");
		scanf("%s", buff1);
		printf("Unesite novo ime za zapakirani file + ekstenzija: ");

		scanf("%s", buff2);

		EncodeToFile(buff1, buff2, 1);
		printf("\nEncoded!\n");

		break;
	case 3:
		printf("Unesite ime zapakiranog file-a:");
		scanf("%s", buff1);
		printf("Unesite novo ime za zapakirani file + ekstenzija: ");

		scanf("%s", buff2);

		DecodeToFile(buff1, buff2, 0);
		printf("\nDecoded!\n");

		break;
	case 4:
		printf("Unesite ime zapakiranog file-a:");
		scanf("%s", buff1);
		printf("Unesite novo ime za zapakirani file + ekstenzija: ");

		scanf("%s", buff2);

		DecodeToFile(buff1, buff2, 1);
		printf("\nDecoded!\n");

		break;
	}

	getchar();
	getchar();

	return 0;
}
