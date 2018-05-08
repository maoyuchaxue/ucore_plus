#include <ulib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <hypercall.h>

#define printf(...)                     fprintf(1, __VA_ARGS__)

char *payload_buffer;


int main(int argc, char **argv)
{
    // hypercall(999,0,0);
	// kAFL_hypercall(9, NULL);
    payload_buffer = shmem_malloc(18 << 20);

    printf("test kAFL:\n");
    printf("payload_buffer addr :%d\n", (uint64_t)payload_buffer);
    // kAFL_hypercall(HYPERCALL_KAFL_ACQUIRE, 0);
	kAFL_hypercall(HYPERCALL_KAFL_GET_PAYLOAD, (uint64_t)payload_buffer);
    printf("get_payload: %s\n", payload_buffer);

    printf("payload_buffer addr :%d\n", (uint64_t)payload_buffer);

    printf("put coverage: \n");
    kAFL_hypercall(HYPERCALL_KAFL_INFO, (uint64_t)payload_buffer);

    printf("test end\n");
    return 0;
}
