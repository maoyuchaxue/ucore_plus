#include <ulib.h>
#include <stdio.h>
#include <string.h>
#include <hypercall.h>

#define printf(...)                     fprintf(1, __VA_ARGS__)

char payload_buffer[16 << 20];

int main(int argc, char **argv)
{
    // hypercall(999,0,0);
	// kAFL_hypercall(9, NULL);
    
    printf("test kAFL:\n");
    // kAFL_hypercall(HYPERCALL_KAFL_ACQUIRE, 0);
	kAFL_hypercall(HYPERCALL_KAFL_GET_PAYLOAD, (uint64_t)payload_buffer);
    printf(payload_buffer);

    printf("test end\n");
    return 0;
}
