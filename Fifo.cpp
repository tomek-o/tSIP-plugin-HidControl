//---------------------------------------------------------------------------

#include "Fifo.h"
#include "ScopedLock.h"
#include <string.h>


FIFO::FIFO(unsigned int entry_count, unsigned int entry_size):
	head(0),
	tail(0),
	entry_count(entry_count),
	entry_size(entry_size)
{
	entries = new struct S_ENTRY*[entry_count];
	for (unsigned int i=0; i<entry_count; i++)
	{
		struct S_ENTRY *entry = new struct S_ENTRY;
		entry->buf = new unsigned char[entry_size];
		entries[i] = entry;
	}
}

FIFO::~FIFO()
{
	for (unsigned int i=0; i<entry_count; i++)
	{
		struct S_ENTRY *entry = entries[i];
		delete[] entry->buf;
		delete entry;
	}
	delete[] entries;
}

int FIFO::put(unsigned char* data, unsigned int len)
{
	int next_head;
	ScopedLock<Mutex> lock(mtx);
	next_head = (++head)%entry_count;
	if (next_head == tail) {
		//overflow
		return -1;
	}
	head = next_head;
	memcpy(entries[head]->buf, data, len);
	entries[head]->len = len;
	return 0;
}

unsigned int FIFO::get(unsigned char** data)
{
	ScopedLock<Mutex> lock(mtx);
	if (head==tail) {
		return 0;
	}
	tail++;
	tail = tail%entry_count;
	struct S_ENTRY *entry = entries[tail];
	*data = entry->buf;
	return entry->len;
}

