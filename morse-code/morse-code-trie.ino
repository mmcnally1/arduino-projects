#include <avr/io.h>
#include <util/delay.h>

unsigned long int _start = 0;
unsigned long int _stop = 0;
unsigned int _timer = 0;
char morse[6];
int morseIndex = 0;
char trie[64] = {'\0','e','t','i','a','n','m','s','u','r','w','d','k',
                 'g','o','h','v','f','\0','l','\0','p','j','b','x',
                 'c','y','z','q','\0','\0','5','4','\0','3','\0','\0',
                 '\0','2','\0','\0','\0','\0','\0','\0','\0','1','6',
                 '\0','\0','\0','\0','\0','\0','\0','7','\0','\0','\0',
                 '8','\0','9','0'};

void Initialize() {
  // Disable interrupts
  cli();

  // Set PB0 (ICP1) as input pin
  DDRB &= ~(1 << DDB0);
   
  // Set input capture select to falling edge
  TCCR1B &= ~(1 << ICES1);

  // Set clock prescaler to 256
  TCCR1B |= (1 << CS12);
  TCCR1B &= ~(1 << CS11);
  TCCR1B &= ~(1 << CS10);

  // Set counter to normal mode
  TCCR1B &= ~(1 << WGM13);
  TCCR1B &= ~(1 << WGM12);
  TCCR1B &= ~(1 << WGM11);
  TCCR1B &= ~(1 << WGM10);

  // Enable input capture interrupt and timer overflow interrupt
  TIMSK1 |= (1 << ICIE1);
  TIMSK1 |= (1 << TOIE0);

  // Clear input capture flag
  TIFR1 |= (1 << ICF1);

  // Initialize serial for printing
  Serial.begin(9600);

  // Enable interrupts
  sei();
}

// Input Capture Interrupt Handler
ISR(TIMER1_CAPT_vect) {
  
  // Clear input capture flag
  TIFR1 |= (1 << ICF1);

  // Rising edge = button released
  if (TCCR1B & (1 << ICES1)) {
    
    // Set input capture edge select to falling edge
    TCCR1B &= ~(1 << ICES1);

    // Measure time between button push and release
    _stop = (_timer * 65536) + TCNT1;
    unsigned long int numTicks = _stop - _start;

    // 30ms to 200ms = dot
    if (numTicks >= 1875 && numTicks <= 12500) {
      morse[morseIndex] = '.';
      morseIndex++;
    }
    
    // 200ms to 500ms = dash
    else if (numTicks > 12500 && numTicks <= 31250) {
      morse[morseIndex] = '-';
      morseIndex++;
    }
    
    // Reset counter variables
    _start = 0;
    _stop = 0;
    _timer = 0;
  }

  // Falling edge = button pushed
  else {
    
    // Record time of button push
    TCCR1B |= (1 << ICES1);
    _start = (_timer * 65536) + TCNT1;
  }
}

// Timer Overflow Interrupt Handler
ISR(TIMER1_OVF_vect) {
  _timer++;

  // 1 second without button push = letter received
  if (((_timer * 65536) - _start) > 65536) {
    morse[morseIndex] = '\0';
    decode();
    morseIndex = 0;
  }
  
  // Clear timer overflow flag
  TIFR0 |= (1 << TOV0);
}

// Use trie data structure to decode input.
// Since there is only dot and dash to decode, trie
// also happens to be a binary search tree.
void decode() {
  int index = 0;
  int trieIndex = 0;
  while (morse[index] != '\0') {
    
    // dot = left child
    if (morse[index] == '.') {
      trieIndex = (trieIndex * 2) + 1;
    }
    
    // dash = right child
    else if (morse[index] == '-') {
      trieIndex = (trieIndex * 2) + 2;
    }
    index++;
  }
  Serial.print(trie[trieIndex]);
}

int main(void) {
  Initialize();
  while(1);
}
