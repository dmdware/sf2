












#ifndef RANDOM_H
#define RANDOM_H

void srand2(unsigned int seed);
unsigned int rand2();

unsigned int rand3();
void srand3(unsigned int seed);

unsigned int rand4();
void srand4(unsigned int s1, unsigned int s2);

#define SRAND	srand3
#define RAND	rand3

struct Rand //TODO using rand3
{
public:
	unsigned int counter;

	void seed(unsigned int s);
	unsigned int next();
};

#endif