#include "functions.h"

void l1_fill(char *arr, int size, int value);
void l1_BinaryRepInt(int n);
void l1_BinaryRepShort(short n);
int l1_or_array(char *arr, int size);
void L1_row_initialize(L1_row *L1row);
void l1_valid_TagPrint ();
void l1_checkInitialization();
short l1_TLBtoFrame(int TLBaddress);
int l1_LRU_victimBlock(int index);
void l1_printLRUBits(int index);
void l1_LRU_Update(int index, char wayPrediction);
void l1_getBlockFromL2(int TLBaddress);
int l1_comparator(int TLBaddress, char wayPrediction, int index);


void l1_fill(char *arr, int size, int value)
{
	for (int i=0; i<size; i++)
	{
		*(arr+i) = value;	
	}
}


void l1_BinaryRepInt(int n)
{
	unsigned int x = 0b10000000000000000000000000000000;
	for (int i=0; i<32; i++)
	{
		printf("%d", (n&x)?1:0);
		x = x>>1;
	}
	printf("\n");
}


void l1_BinaryRepShort(short n)
{
	unsigned short x = 0b1000000000000000;
	for (int i=0; i<16; i++)
	{
		printf("%d", (n&x)?1:0);
		if (i%4==3)
			printf("\n");
		x = x>>1;
	}
}

int l1_or_array(char *arr, int size)
{	
	int ans = 0;
	for (int i=0; i<size; i++)
	{
		ans = ans | *(arr+i);
	}
	return ans;
}

/*
    Input of function - physical address.
    Purpose of the function - to calculate the index for l1.
    Output/Result of function - index.
*/
int l1_get_index(int TLBaddress){
	int index = TLBaddress & 0x3f0 >> L1_OFFSET;
	return index;
}

/*
    Input of function - physical address.
    Purpose of the function - to calculate the offset for l1.
    Output/Result of function - offset.
*/
int l1_get_offset(int TLBaddress){
	int offset = TLBaddress & 0xf;
	return offset;
}

/*
    Input of function - physical address.
    Purpose of the function - to calculate the tag bits from the physical address.
    Output/Result of function - tag bits.
*/
short l1_TLBtoFrame(int TLBaddress)
{
	short framebits = TLBaddress >> 10;
	return framebits;
}


/*
    Input of function - A row of L1 Cache
    Purpose of the function - Initialize the row of the L1 Cache.
    Output/Result of function - Initialized row to zero.
*/
void L1_row_initialize(L1_row *L1row)
{
	l1_fill((char *)L1row, sizeof(*L1row),0);
}

/*
    Purpose of the function - Initialize L1 Instruction Cache.
    Output/Result of function - L1 Instruction Cache Initialized to all zeroes.
*/
L1_struct* l1_instruction_initialize(){
	L1_struct* l1_instruction = malloc(sizeof(L1_struct));
	for (int i =0; i<ENTRIES; i++)
	{
		L1_row_initialize(&(l1_instruction->L1row[i]));
	}
	return l1_instruction;
}

/*
    Purpose of the function - Initialize L1 Data Cache.
    Output/Result of function - L1 Data Cache Initialized to all zeroes.
*/
L1_struct* l1_data_initialize(){
	L1_struct* l1_data = malloc(sizeof(L1_struct));
	for (int i =0; i<ENTRIES; i++)
	{
		L1_row_initialize(&(l1_data->L1row[i]));
	}
	return l1_data;
}

void l1_valid_TagPrint ()
{
	for (int i=0; i<ENTRIES; i++)
	{
		printf(" : %d", l1->L1row[i].valid_Tag[0]);
		printf(" : %d", l1->L1row[i].valid_Tag[1]);
		printf(" : %d", l1->L1row[i].valid_Tag[2]);
		printf(" : %d\n", l1->L1row[i].valid_Tag[3]);
	}
}

void l1_checkInitialization()
{
	for (int i=0; i<ENTRIES; i++)
	{
		printf("%d\n", l1_or_array((char *)&(l1->L1row[i]), sizeof(l1->L1row[i])));
	}
}

void l1_getBlockFromL2(int TLBaddress)
{
	srand(time(0));
	for (int i=0; i<L1_BLOCK; i++)
		bus16B[i] = rand();
}



/*
    Input of function - index of a row in cache
    Purpose of the function - to find the lru block in l1 cache
    Output/Result of function - lru block way number
*/
int l1_LRU_victimBlock(int index)
{
	short lruBits = l1->L1row[index].LRUBits;
	if ((lruBits & 0b1111000000000000) == 0)
		return 0;
	else if ((lruBits & 0b0000111100000000) == 0)
		return 1;
	else if ((lruBits & 0b0000000011110000) == 0)
		return 2;
	else 
		return 3;
}

void l1_printLRUBits(int index)
{
	l1_BinaryRepShort(l1->L1row[index].LRUBits);
}


/*
    Input of function - index of the cache and way number of the hit block.
    Purpose of the function - make the row bits for the way number all 1's and column bits for way number all 0's.
*/
void l1_LRU_Update(int index, char wayPrediction)  //on hit
{
	switch(wayPrediction)
	{
		case 0:
			l1->L1row[index].LRUBits |= 0b1111000000000000;
			l1->L1row[index].LRUBits &= 0b0111011101110111;
			break;
		case 1:
			l1->L1row[index].LRUBits |= 0b0000111100000000;
			l1->L1row[index].LRUBits &= 0b1011101110111011;
			break;
		case 2:
			l1->L1row[index].LRUBits |= 0b0000000011110000;
			l1->L1row[index].LRUBits &= 0b1101110111011101;
			break;
		case 3:
			l1->L1row[index].LRUBits |= 0b0000000000001111;
			l1->L1row[index].LRUBits &= 0b1110111011101110;
			break;
	}
}

/*
    Input of function - physical_address and data to write into l1.
    Purpose of the function - writing data to l1.
    Output/Result of function - data written to l1 cache and added block on bus for write through to l2.
*/
void l1_write_to_l1(int physical_address, char data){
	int index = l1_get_index(physical_address);
	int offset = l1_get_offset(physical_address);

	int way_no = l1->L1row[index].wayPrediction;

	l1->L1row[index].data[L1_BLOCK*way_no + offset] = data;

	for (int i =0; i<L1_BLOCK; i++)
	{
		bus16B[i] = l1->L1row[index].data[L1_BLOCK*way_no + i];
	}

	return;
}


void l1_print_cache(L1_struct *l1){
	for(int i=0; i<ENTRIES; i++){
		printf("index = %d, wayprediction = %d\n", i, l1->L1row[i].wayPrediction);
	}
}

/*
    Input of function - physical_address and index to handle cache miss.
    Purpose of the function - getting data from bus16B which has the data from l2.
    Output/Result of function - bus data is written in the repective row and lru block.
*/
void l1_cacheMiss(int TLBaddress, int index)
{
	short framebits = l1_TLBtoFrame(TLBaddress);
	framebits = framebits | 0x8000;
	l1_getBlockFromL2(TLBaddress);
	//In the cpu driver code, we call l2_read_to_l1 function which transfers the data on bus16B. So no need to call this function.
	int victimBlock = l1_LRU_victimBlock(index);
	l1->L1row[index].valid_Tag[victimBlock] = framebits;
	for (int i =0; i<L1_BLOCK; i++)
	{
		l1->L1row[index].data[L1_BLOCK*victimBlock + i] = bus16B[i];
	}
	l1->L1row[index].wayPrediction = victimBlock;
}


/*
    Input of function - physical_address and way number and index to indentify a particular block in l1.
    Purpose of the function - check if that block has the data or not.
    Output/Result of function - if data present return way number.
*/
int l1_comparator(int TLBaddress, char wayPrediction, int index)
{
	short frameBits = l1_TLBtoFrame(TLBaddress);
	short valid_frameBits = frameBits | 0x8000;
	short valid_Tag = 0;
	
	switch(wayPrediction)
	{
		case 0:
			valid_Tag = l1->L1row[index].valid_Tag[0];
			break;
		case 1:
			valid_Tag = l1->L1row[index].valid_Tag[1];
			break;
		case 2:
			valid_Tag = l1->L1row[index].valid_Tag[2];
			break;
		case 3:
			valid_Tag = l1->L1row[index].valid_Tag[3];
			break;
		default:
			printf("Invalid wayPrediction Number.\n");	
	}
	if ((valid_Tag & 0x8000) == 0)
		return -1; //invalid 
	else if (valid_Tag == valid_frameBits)
	{	
		l1_LRU_Update(index, wayPrediction);
		return wayPrediction; //hit
	}
	else return -2;  //cache miss
}


/*
    Input of function - physical_address and index and offset.
    Purpose of the function - search cache for hit or miss.
    Output/Result of function - if miss return 0, else return data.
*/
char l1_search_cache(int TLBaddress, int index, int offset)
{
	int return_comparator;
	return_comparator = l1_comparator(TLBaddress, l1->L1row[index].wayPrediction, index);

	total_access_time += l1_cache_lookup_time;

	if (return_comparator >= 0)
	{
		// cache hit
		return  l1->L1row[index].data[L1_BLOCK*return_comparator + offset];
	}

	// checks done in parallel in hardware
	return_comparator = l1_comparator(TLBaddress, (l1->L1row[index].wayPrediction+1)%4, index);
	if (return_comparator >= 0)
	{
		// cache hit
		return  l1->L1row[index].data[L1_BLOCK*return_comparator + offset];
	}
	return_comparator = l1_comparator(TLBaddress, (l1->L1row[index].wayPrediction+2)%4, index);
	if (return_comparator >= 0)
	{
		// cache hit
		return  l1->L1row[index].data[L1_BLOCK*return_comparator + offset];
	}
	return_comparator = l1_comparator(TLBaddress, (l1->L1row[index].wayPrediction+3)%4, index);
	if (return_comparator >= 0)
	{
		// cache hit
		return  l1->L1row[index].data[L1_BLOCK*return_comparator + offset];
	}

	return 0;
}
