//---------------------------------------------------------------------------

#ifndef FifoH
#define FifoH
//---------------------------------------------------------------------------

#include "Mutex.h"

class FIFO
{
	int head, tail;
	const unsigned int entry_count, entry_size;
	struct S_ENTRY
	{
		unsigned char *buf;
		unsigned int len;
	} **entries;
	Mutex mtx;
public:
	FIFO(unsigned int entry_count, unsigned int entry_size);
	~FIFO();
	/** \return 0 on success, non-zero on overflow
	*/
	int put(unsigned char* data, unsigned int len);
	/** \param data pointer to receive pointer to data buffer
		\return number of samples, data may be invalid if number of samples = 0
	*/
	unsigned int get(unsigned char** data);
};

#endif
