#include <iostream>

// Sample inheritance structure for testing

class Animal {
};

class Mammal : public Animal {
};

class Bird : public Animal {
};

class Bat : public Mammal {
};

class Ostrich : public Bird {
};

class Whale : public Mammal {
};

class Pegasus : public Mammal, public Bird {
};
