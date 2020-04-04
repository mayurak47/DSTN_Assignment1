#include "functions.h"

l2_address l2_get_l2_address_from_physical_address(int physical_address);
int l2_get_physical_address_from_l2_address(l2_address l2_addr);
int l2_get_victim_block(int set_index);
void l2_update_lru_counter(int set_index, int mru_block_index);
void l2_get_block_from_mm(l2_address addr, int victim_block);
void l2_write_dirty_block_back_to_mm(l2_address l2_addr, int victim_block);

l2_address l2_get_l2_address_from_physical_address(int physical_address) {
	l2_address l2_addr;
	l2_addr.offset = (physical_address & 0x001f);
	l2_addr.index = (physical_address & 0x0fe0) >> 5;
	l2_addr.tag = (physical_address & 0xfffff000) >> 12;
	
	return l2_addr;
}

int l2_get_physical_address_from_l2_address(l2_address l2_addr) {
	int physical_address = 0x0;
	physical_address |= l2_addr.offset;
	physical_address |= (l2_addr.index << 5);
	physical_address |= (l2_addr.tag << 12);
	
	return physical_address;
}

int l2_get_victim_block(int set_index) {
	for(int i=0;i<NO_OF_L2_WAYS;i++) {
		if(l2->l2_sets[set_index].l2_entries[i].valid == 0)
			return i;
	}
	
	for(int i=0;i<NO_OF_L2_WAYS;i++) {
		if(l2_lru_counter.lru_sets[set_index].lru_bits[i] == 0)
			return i;
	}
	return -1;
}

void l2_update_lru_counter(int set_index, int mru_block_index) {
	l2_lru_counter.lru_sets[set_index].lru_bits[mru_block_index] = 0x7;
	for(int i=0;i<NO_OF_L2_WAYS;i++) {
		if(i == mru_block_index)
			continue;
		
		if(l2_lru_counter.lru_sets[set_index].lru_bits[i] > 1)
			l2_lru_counter.lru_sets[set_index].lru_bits[i]--;
		else
			l2_lru_counter.lru_sets[set_index].lru_bits[i]=0;
	}
	return;
}

void l2_get_block_from_mm(l2_address addr, int victim_block) {
	int physical_address = l2_get_physical_address_from_l2_address(addr);

	total_access_time += mm_to_l2_write_time;

	mm_get_data_from_frame(physical_address);
	return;
}

void l2_write_dirty_block_back_to_mm(l2_address l2_addr, int victim_block) {
	int physical_address = l2_get_physical_address_from_l2_address(l2_addr);
	
	total_access_time += l2_to_mm_write_time;

	for(int i=0;i<L2_BLOCK;i++)
		bus32B[i] = l2->l2_sets[l2_addr.index].l2_entries[victim_block].data[i];
	mm_write_to_mm(physical_address, &main_memory);
	l2->l2_sets[l2_addr.index].l2_entries[victim_block].dirty = 0;
}

int l2_service_cache_miss(int physical_address) {
	l2_address l2_addr = l2_get_l2_address_from_physical_address(physical_address);
	
	int victim_block = l2_get_victim_block(l2_addr.index);
	
	if(l2->l2_sets[l2_addr.index].l2_entries[victim_block].dirty)
		l2_write_dirty_block_back_to_mm(l2_addr, victim_block);
	
	l2_get_block_from_mm(l2_addr, victim_block);
	
	l2_entry new_entry;
	new_entry.valid = 1;
	new_entry.dirty = 0;
	new_entry.tag = l2_addr.tag;
	for(int i=0;i<L2_BLOCK;i++)
		new_entry.data[i] = bus32B[i];
	
	l2->l2_sets[l2_addr.index].l2_entries[victim_block] = new_entry;
	l2_update_lru_counter(l2_addr.index, victim_block);
	// l2_read_to_l1(l2_get_physical_address_from_l2_address(l2_addr));
	
	return victim_block;
}

//returns way no
int l2_search_cache(int physical_address) {
	l2_address l2_addr = l2_get_l2_address_from_physical_address(physical_address);
	l2_set l2_set = l2->l2_sets[l2_addr.index];
	int tag = l2_addr.tag;

	for(int i=0;i<NO_OF_L2_WAYS;i++) {
		if(l2_set.l2_entries[i].valid == 1 && l2_set.l2_entries[i].tag == tag) {
			//L2 Cache hit
			l2_update_lru_counter(l2_addr.index, i);
			return i;
		}
	}
	return -1;
}

void l2_write_from_l1_to_l2(int physical_address){
   
	l2_address l2_addr = l2_get_l2_address_from_physical_address(physical_address);
   
	int wayNo = l2_search_cache(physical_address);
	//-1 Not really required because inclusive cache
	
	total_access_time += l1_to_l2_write_time;

	l2->l2_sets[l2_addr.index].l2_entries[wayNo].dirty = 1;

	if(l2_addr.offset < L2_BLOCK/2) {
		for(int i=0;i<L2_BLOCK/2;i++)
			l2->l2_sets[l2_addr.index].l2_entries[wayNo].data[i] = bus16B[i]; 
	} else {
		for(int i=0;i<L2_BLOCK/2;i++)
			l2->l2_sets[l2_addr.index].l2_entries[wayNo].data[i+L2_BLOCK/2] = bus16B[i];
	}
    
   return;
}

void l2_read_to_l1(int physical_address, int wayNo) {
	
	//Transfer data from l2 to bus16B
	l2_address l2_addr = l2_get_l2_address_from_physical_address(physical_address);
	
	total_access_time += l2_to_l1_write_time;

	int offset = l2_addr.offset;
	if(offset < L2_BLOCK/2) {
		for(int i=0;i<L2_BLOCK/2;i++)
			bus16B[i] = l2->l2_sets[l2_addr.index].l2_entries[wayNo].data[i];
	} else {
		for(int i=L2_BLOCK/2;i<L2_BLOCK;i++)
			bus16B[i] = l2->l2_sets[l2_addr.index].l2_entries[wayNo].data[i];
	}
}


void l2_initialize() {
	l2 = malloc(sizeof(l2_cache));
	for(int i=0;i<SETS;i++) {
		for(int j=0;j<NO_OF_L2_WAYS;j++) {
			l2->l2_sets[i].l2_entries[j].valid = 0;
			l2->l2_sets[i].l2_entries[j].dirty = 0;
			l2->l2_sets[i].l2_entries[j].tag = 0;
			
			for(int k=0;k<L2_BLOCK;k++)
				l2->l2_sets[i].l2_entries[j].data[k] = rand()%26 + 65;
			l2->l2_sets[i].l2_entries[j].data[L2_BLOCK+1] = '\0';
				
			l2_lru_counter.lru_sets[i].lru_bits[j]=0;
		}
	}
}

void l2_terminate() {
	for(int i=0;i<SETS;i++) {
		for(int j=0;j<NO_OF_L2_WAYS;j++) {
			if(l2->l2_sets[i].l2_entries[j].dirty==1) {
				l2_address l2_addr;
				l2_addr.tag=l2->l2_sets[i].l2_entries[j].tag;
				l2_addr.index=i;
				l2_addr.offset=0;
				l2_write_dirty_block_back_to_mm(l2_addr, j);
			}
		}
	}
}

void l2_spaceRemover(char *s)
{
	char *d = s;
	while (*d!=0)
	{
		if (*d==' ')
		{
			d++;
			continue;
		}
		*s = *d;
		s++;
		d++;
	}
	*s = *d;
}

// int main() {
	
// 	//~ int f = 0x7fff8000;
// 	//~ L2_address l2addr = l2_get_l2_address_from_physical_address(f);
// 	//~ printf("%x", l2_get_physical_address_from_l2_address(l2addr));
	
// 	l2_initialize();

// 	FILE * fp = fopen("VORTEX.txt", "r");
// 	char input[50];
// 	int cnt=1;
// 	while (fgets(input, 50, fp)!=NULL)
// 	{
// 		l2_spaceRemover(input);
// 		int tlbaddress =strtol(input, NULL, 16); 
// 		if(cnt)
// 			l2_read_to_l1(tlbaddress);
// 		else
// 			l2_write_from_l1_to_l2(tlbaddress);
// 		cnt++;
// 		cnt=cnt%2;
// 	}
// 	// checkInitialization();
// 	printf("Total Cache Hits: %d\n", hit);
// 	printf("Total Cache Miss: %d\n", miss);
// 	printf("Hit ratio : %f\n", (float)(hit)/(hit + miss));

// 	return 0;
// }
