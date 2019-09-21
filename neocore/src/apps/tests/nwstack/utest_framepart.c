#include "utest_suite.h"
#include "framepart.h"



// size_t x = heap_size
static void create_test()
{
	framePart_s* fp = NULL;
	bool res;

	umsg_line("Frame part");

	framePart_t type = PPDU_HEADER;
	uint8_t part_data[10] = {0,1,2,3,4,5,6,7,8,9};
	uint8_t part_len = sizeof(part_data);

	res =FP_create(fp, type, part_data, part_len);

	umsg("FP_create", "Frame created", 
		(res == true) &&
		(fp != NULL)
		);

	umsg("FP_create" , "Type", fp->type == type);
	umsg("FP_create" , "Length", fp->part_len == part_len);

	umsg("FP_create" , "Length", 
		memory_compare((char*)fp->part_data, (char*)part_data, part_len));

	umsg("FP_create", "Last", fp->last == NULL);
	umsg("FP_create", "Next", fp->next == NULL);

	FP_delete(fp);	
}

static void delete_test()
{
	bool memory_ok;
	framePart_s* fp = NULL;
	bool res = false;

	umsg_line("Frame part");


	size_t before_malloc = heap_size();

	//res =FP_create(fp);

	umsg("Frame part", "FP_create", 
		(res == true) &&
		(fp != NULL)
		);


	umsg("framepart", "test 1", true);

	memory_ok = before_malloc == heap_size();
}

void run_utest_framepart(void)
{
	create_test();
	delete_test();
}