#include <ulib.h>
#include <stdio.h>
#include <string.h>
#include <hypercall.h>

int main(int argc, char **argv)
{
    char payload_buffer[100];
    // hypercall(999,0,0);
	// kAFL_hypercall(9, NULL);
    
    cprintf("test kAFL:\n");
    // kAFL_hypercall(HYPERCALL_KAFL_ACQUIRE, 0);
	kAFL_hypercall(HYPERCALL_KAFL_GET_PAYLOAD, (uint64_t)payload_buffer);
    cprintf(payload_buffer);

    cprintf("test end\n");
    return 0;
}
