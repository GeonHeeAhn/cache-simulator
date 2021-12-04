/*
 * cache.c
 *
 * 20493-01 Computer Architecture
 * Term Project on Implentation of Cache Mechanism
 *
 * Skeleton Code Prepared by Prof. HyungJune Lee
 * Nov 15, 2021
 *
 */


#include <stdio.h>
#include <string.h>
#include "cache_impl.h"

extern int num_cache_hits;
extern int num_cache_misses;

extern int num_bytes;
extern int num_access_cycles;

extern int global_timestamp;

cache_entry_t cache_array[CACHE_SET_SIZE][DEFAULT_CACHE_ASSOC];
int memory_array[DEFAULT_MEMORY_SIZE_WORD];


/* DO NOT CHANGE THE FOLLOWING FUNCTION */
void init_memory_content() {
    unsigned char sample_upward[16] = { 0x001, 0x012, 0x023, 0x034, 0x045, 0x056, 0x067, 0x078, 0x089, 0x09a, 0x0ab, 0x0bc, 0x0cd, 0x0de, 0x0ef };
    unsigned char sample_downward[16] = { 0x0fe, 0x0ed, 0x0dc, 0x0cb, 0x0ba, 0x0a9, 0x098, 0x087, 0x076, 0x065, 0x054, 0x043, 0x032, 0x021, 0x010 };
    int index, i = 0, j = 1, gap = 1;

    for (index = 0; index < DEFAULT_MEMORY_SIZE_WORD; index++) {
        memory_array[index] = (sample_upward[i] << 24) | (sample_upward[j] << 16) | (sample_downward[i] << 8) | (sample_downward[j]);
        if (++i >= 16)
            i = 0;
        if (++j >= 16)
            j = 0;

        if (i == 0 && j == i + gap)
            j = i + (++gap);

        printf("mem[%d] = %#x\n", index, memory_array[index]);
    }
}

/* DO NOT CHANGE THE FOLLOWING FUNCTION */
void init_cache_content() {
    int i, j;

    for (i = 0; i < CACHE_SET_SIZE; i++) {
        for (j = 0; j < DEFAULT_CACHE_ASSOC; j++) {
            cache_entry_t* pEntry = &cache_array[i][j];
            pEntry->valid = 0;
            pEntry->tag = -1;
            pEntry->timestamp = 0;
            strcpy(pEntry->data,"");
        }
    }
}

/* DO NOT CHANGE THE FOLLOWING FUNCTION */
/* This function is a utility function to print all the cache entries. It will be useful for your debugging */
void print_cache_entries() {
    int i, j, k;

    for (i = 0; i < CACHE_SET_SIZE; i++) {
        printf("[Set %d] ", i);
        for (j = 0; j < DEFAULT_CACHE_ASSOC; j++) {
            cache_entry_t* pEntry = &cache_array[i][j];
            printf("V: %d Tag: %#x Time: %d Data: ", pEntry->valid, pEntry->tag, pEntry->timestamp);
            for (k = 0; k < DEFAULT_CACHE_BLOCK_SIZE_BYTE; k++) {
                printf("%#x(%d) ", pEntry->data[k], k);
            }
            printf("\t");
        }
        printf("\n");
    }
}

void decimal_to_binary(int decimal, int binary[]) {
    int position = 0;
    int temp;

    while (1)
    {
        binary[position] = decimal % 2;    // 2로 나누었을 때 나머지를 배열에 저장
        decimal = decimal / 2;             // 2로 나눈 몫을 저장

        position++;    // 자릿수 변경

        if (decimal == 0)    // 몫이 0이 되면 반복을 끝냄
            break;
    }

    for (int i = 0; i < 5 / 2; i++) {
        temp = binary[i];
        binary[i] = binary[(5 - 1) - i];
        binary[(5 - 1) - i] = temp;
    }
}

void tag_and_set(int tag[], int set[], int binary_block_address[]) {
    for (int i = 0; i < sizeof(*tag) / sizeof(int); i++) {
        tag[i] = binary_block_address[i];
    }

    for (int j = 0; j < sizeof(*set) / sizeof(int); j++) {
        set[j] = binary_block_address[(sizeof(*tag) / sizeof(int) + j)];
    }

}

int binary_to_decimal(int binary[]) {
    int decimal = 0;
    int position = 0;
    for (int i = sizeof(*binary) / sizeof(int) - 1; i >= 0; i--)
    {
        if (binary[i] == 1)
            decimal += 1 << position;

        position++;
    }

    return decimal;
}

int check_cache_data_hit(void* addr, char type) {
    /* Fill out here */
    int byte_address = *(int*)addr; //여기 unsigned long int로 해야할까?
    int block_address;
    int binary_block_addr[5] = { 0, };
    int set_decimal;
    int tag_decimal;
    int byte_of_block = byte_address % DEFAULT_CACHE_BLOCK_SIZE_BYTE;

    //direct mapped cache
    block_address = byte_address / 8;
    //block address를 이진수로 변환
    decimal_to_binary(block_address, binary_block_addr);

    printf("binary_block_addr:\n");
    for (int i = 0; i < 5; i++) {
        printf("%d", binary_block_addr[i]);
    }
    printf("\n");

    //tag와 set 변수 선언
    int direct_tag[3] = { 0 };
    int direct_set[2] = { 0 };

    tag_and_set(direct_tag, direct_set, binary_block_addr);

    //캐시 배열 접근 위해 이진수를 십진수로 변환
    set_decimal = binary_to_decimal(direct_set);
    tag_decimal = binary_to_decimal(direct_tag);

    cache_entry_t* pEntry = &cache_array[set_decimal][0]; 

    //valid == 0일 경우 cache 존재x 이므로 -1반환
     /* Return the data */
    if (pEntry->valid == 0) {
        return -1;
    }
    else {
        if (type == 'b') {
            return pEntry->data[byte_of_block];
        }
        else if (type == 'h') {
            char data[10] = { 0, };
            if (byte_of_block == 8) {
                strcpy(data, pEntry->data[byte_of_block-1]);
                strcat(data, pEntry->data[byte_of_block]);
            }
            else {
                strcpy(data, pEntry->data[byte_of_block]);
                strcat(data, pEntry->data[byte_of_block + 1]);
            }
            return data;
        }
        else {
            char data[10] = { 0, };
            if (byte_of_block <= 3) {
                strcpy(data, pEntry->data[0]);
                for (int i = 1; i < 4; i++) {
                    strcat(data, pEntry->data[i]);
                }
            }
            else {
                strcpy(data, pEntry->data[4]);
                for (int i = 5; i < 8; i++) {
                    strcat(data, pEntry->data[i]);
                }
            }
        }
    }
}

int find_entry_index_in_set(int cache_index) {
    int entry_index;

    //case of direct mapped cache
    /* Check if there exists any empty cache space by checking 'valid' */
    for (int i = 0; i < DEFAULT_CACHE_ASSOC; i++) {
        cache_entry_t* pEntry = &cache_array[cache_index][i];
        if (pEntry->valid == 0) {
            entry_index = i;
            break;
        }
        else entry_index = -1;
    }


    /* Otherwise, search over all entries to find the least recently used entry by checking 'timestamp' */
    int min_timeStamp;
    int min_index;

    if (entry_index == -1) {
        for (int i = 0; i < DEFAULT_CACHE_ASSOC; i++) {
            cache_entry_t* pEntry = &cache_array[cache_index][i];
            if (i == 0) {
                min_timeStamp = pEntry->timestamp;
                min_index = 0;
            }
            if (min_timeStamp > pEntry->timestamp) {
                min_timeStamp = pEntry->timestamp;
                min_index = i;
            }
        }
        entry_index = min_index;
    }


    return entry_index;
}

int access_memory(void* addr, char type) {

    /* Fetch the data from the main memory and copy them to the cache */
    /* void *addr: addr is byte address, whereas your main memory address is word address due to 'int memory_array[]' */
    //byte address * block size / word size 한 값을 type에 따라 메모리에서 가져오면 됨 
   //메인 메모리에 어떻게 접근할건지
    //cache 인덱스를 갖고 find_entry_index_in_set에 접근, 
    //위 함수에서 entry_index 반환
    //index가 2bit
    char memory_data[DEFAULT_CACHE_BLOCK_SIZE_BYTE];
    char word1[10] = { 0, };
    char word2[10] = { 0, };
    int cache_index;
    int entry_index;
    int byte_address = *(int*)addr;
    int binary_block_address[5] = { 0 };
    int mem_num = byte_address * DEFAULT_CACHE_BLOCK_SIZE_BYTE / 4;
    //한 블록에서 몇번째 바이트인지
    int byte_of_block;
    //direct mapped cache
    //set num == cache index
    decimal_to_binary(byte_address / DEFAULT_CACHE_BLOCK_SIZE_BYTE, binary_block_address);

    int binary_cache_index[2] = { 0 };
    for (int i = 0; i < 2; i++) {
        binary_cache_index[i] = binary_block_address[i + 3];
    }

    //cache index 값 얻음
    cache_index = binary_to_decimal(binary_cache_index);

    /* You need to invoke find_entry_index_in_set() for copying to the cache */
    entry_index = find_entry_index_in_set(cache_index);

    /* Return the accessed data with a suitable type */
    //블록 쪼개기
   // string word1_s;
    //string_word2_s;
    if (mem_num % 2 != 0) {
        sprintf(word1, "%#x", memory_array[mem_num - 1]);
        sprintf(word2, "%#x", memory_array[mem_num]);
        printf("mem[%d] : %#x\n\n", mem_num - 1, memory_array[mem_num - 1]);
        printf("mem[%d] : %#x\n\n", mem_num, memory_array[mem_num ]);
    }
    else {
        sprintf(word1, "%#x", memory_array[mem_num]);
        sprintf(word2, "%#x", memory_array[mem_num + 1]);
        printf("mem[%d] : %#x\n\n", mem_num, memory_array[mem_num]);
        printf("mem[%d] : %#x\n\n", mem_num + 1, memory_array[mem_num + 1]);

    }
    printf("word1: %s\n\n", word1);
    printf("word2: %s\n\n", word2);
    //각각의 메모리값에서 타입에 따른 데이터 필요

    //데이터를 집어넣을 곳 선택
    cache_entry_t* pEntry = &cache_array[cache_index][entry_index];
    
    //10자리 문자열 -> 다섯개로 쪼개서 뒤에 네개 저장해보자 - 역순으로
    int k = 0;
    for (int i = 9; i >= 0; i -= 2) {
        strcat(strcat(strcat(pEntry->data[k++], "0x"), word1[i - 1]), word1[i]);
        if (k == WORD_SIZE_BYTE) break;
    }
    for (int i = 9; i >= 0; i -= 2) {
        strcat(strcat(strcat(pEntry->data[k++], "0x"), word2[i - 1]), word2[i]);
        if (k == DEFAULT_CACHE_BLOCK_SIZE_BYTE) break;
    }

    byte_of_block = byte_address % DEFAULT_CACHE_BLOCK_SIZE_BYTE;
    if (type == 'b') {
        //b는 mem[n]
        printf("data from cache: %s\n\n", pEntry->data[byte_of_block]);
        return strtol(pEntry->data[byte_of_block], NULL, 16);
    }
    else if (type == 'h') {
        //h는 half word니까 mem[n-n+1]
        if (byte_of_block <= 3) {
            printf("data from cache: %s\n\n", strcat(strcat(strcat(strcat("0x", word1[10 - 2 * byte_of_block - 3]), word1[10 - 2 * byte_of_block - 2]), word1[10 - 2 * byte_of_block - 1]), word1[10 - 2 * byte_of_block]));
            return strol(strcat(strcat(strcat(strcat(strcat(pEntry->data[byte_of_block], "0x"), word1[10 - 2 * byte_of_block - 3]), word1[10 - 2 * byte_of_block - 2]), word1[10 - 2 * byte_of_block - 1]), word1[10 - 2 * byte_of_block]), NULL, 16);
        }
        else {
            printf("data from cache: %s\n\n", strcat(strcat(strcat(strcat("0x", word2[10 - 2 * byte_of_block - 3]), word2[10 - 2 * byte_of_block - 2]), word2[10 - 2 * byte_of_block - 1]), word2[10 - 2 * byte_of_block]));
            return strol(strcat(strcat(strcat(strcat(strcat(pEntry->data[byte_of_block], "0x"), word2[10 - 2 * byte_of_block - 3]), word2[10 - 2 * byte_of_block - 2]), word2[10 - 2 * byte_of_block - 1]), word2[10 - 2 * byte_of_block]), NULL, 16);
        }
    }
    else {
        //w는 mem[n-n+3]
        if (byte_of_block <= 3) {
            printf("data from cache: %s\n\n", word1);
            return strol(word1, NULL, 16);
        }
        else {
            printf("data from cache: %s\n\n", word2);
            return strol(word2, NULL, 16);;
        }
    }

}
