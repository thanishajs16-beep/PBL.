#define MEMORY_SIZE 100

static int memory[MEMORY_SIZE];

void memory_write(int address, int value)
{
    if (address < 0 || address >= MEMORY_SIZE) {
        return;
    }

    memory[address] = value;
}

int memory_read(int address)
{
    if (address < 0 || address >= MEMORY_SIZE) {
        return -1;
    }

    return memory[address];
}
