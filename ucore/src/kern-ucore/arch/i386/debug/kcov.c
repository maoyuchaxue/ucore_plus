#define notrace __attribute__((no_instrument_function))
void notrace __sanitizer_cov_trace_pc(void)
{

}

