#include "Trace.h"

#include "Controller.h"

extern TraceParser *initTraceParser(const char * mem_file);
extern bool getRequest(TraceParser *mem_trace);

extern Controller *initController();
extern unsigned ongoingPendingRequests(Controller *controller);
extern bool send(Controller *controller, Request *req);
extern void tick(Controller *controller);

int main(int argc, const char *argv[])
{	
    if (argc != 2)
    {
        printf("Usage: %s %s\n", argv[0], "<mem-file>");
        return 0;
    }

    // Initialize a CPU trace parser
    TraceParser *mem_trace = initTraceParser(argv[1]);
    long int size = 0;
    
	char c;
    FILE* fp = fopen(argv[1], "rb");
    for (c = getc(fp) ; c != EOF ; c = getc(fp)){
        if (c == '\n'){ // Increment count if this character is newline
            size++;
        }
    }
    // closing the file
    fclose(fp);
    
	printf("Number of Lines: %ld\n", size);

    // Initialize a Controller
    Controller *controller = initController();
	printf("Passes Controller\n");
    //printf("%u\n", controller->bank_shift);
    //printf("%lu\n", controller->bank_mask);

    uint64_t cycles = 0;
    uint64_t counter = 0;
    uint64_t conflicts = 0;
    int arr[size*1000];

	//printf("Before WHILE loop\n");

    bool stall = false;
    bool end = false;

	int count = 0;
	int prev_bank = 0;

	while (!end || ongoingPendingRequests(controller))
    {
		//printf("I got in the WHILE loop\n");
        if (!end && !stall)
        {
			//printf("I got in the 1st IF\n");

            end = !(getRequest(mem_trace));
			//printf("%d\n", mem_trace->cur_req->bank_id);
			if (mem_trace->cur_req->bank_id != 0 && mem_trace->cur_req->bank_id == prev_bank) {
				conflicts++;
			}
			prev_bank = mem_trace->cur_req->bank_id;
			//printf("Got passed CONFLICT\n");
            //size++;
        }

        if (!end)
        {
            stall = !(send(controller, mem_trace->cur_req));
        }

        tick(controller);
        ++cycles;
		count++;
    }


    free(controller->bank_status);
    free(controller->waiting_queue);
    free(controller->pending_queue);
    free(controller);
	printf("Iteration: %d\n", count);
    printf("Total Bank Conflicts: ""%"PRIu64"\n", conflicts);
    printf("End Execution Time: ""%"PRIu64"\n", cycles);
}
