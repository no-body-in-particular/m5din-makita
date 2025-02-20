#define SWAP_NIBBLES(x) ((x & 0x0F) << 4 | (x & 0xF0) >> 4)
template < int m_pin > class Makita {
       public:
       Makita(){
        init();
       }

       void init(){
          m_timestamp=0;
          enable_extended=false;
          reset_rom();
          set_health(100);
          set_overheat(0);
          set_overcurrent(0);

          pinMode (m_pin, INPUT) ;
          digitalWrite (m_pin, LOW) ;
       }

 
       //resets ROM to default values taken from a chinese battery
       void reset_rom(){
        uint8_t empty_rom[32]={ 0xf1, 0x36, 0xb6, 0xc3, 0x18, 0x58, 0x00, 0x00, 0x94, 0x94, 0x40, 0x21, 0x01, 0x80, 0x02, 0x0a, 0x43, 0xd0, 0x8e, 0x1b, 0xf0, 0x66, 0x00, 0x03, 0x02, 0x02, 0x00, 0x00, 0x00, 0x10, 0x02, 0x73};
        memcpy(m_rom,empty_rom,32);
       }
 
       /**
        * Check for reset signal. Return true(1) if reset was detected and
        * presence was signaled, otherwise false(0).
        * @return true(1) on reset and presence, otherwise false(0).
        */
       bool reset() {
         pinMode (m_pin, INPUT) ;

         // Check reset start
         if (m_timestamp == 0) {
           if (!digitalRead(m_pin)) m_timestamp = micros();
           return (false);
         }
 
         // Check reset pulse stop
         if (!digitalRead(m_pin)) return (false);
 
         // Check reset pulse width
         if (micros() - m_timestamp < 300) {
           m_timestamp = 0;
           return (false);
         }
 
         // Generate presence signal
         delayMicroseconds(35);
         pinMode (m_pin, OUTPUT) ;
         delayMicroseconds(100);
         pinMode (m_pin, INPUT) ;

         // Wait for possible presence signals from other devices
         while (!digitalRead(m_pin));
         m_timestamp = 0;
 
         delayMicroseconds(200);
         return (true);
       }
 

       uint8_t read(uint8_t bits = 8) {
         uint8_t bitMask;
         uint8_t r = 0;

         pinMode(m_pin,INPUT);
 
         noInterrupts();
 
         for (bitMask = 0x01; bitMask; bitMask <<= 1) {
           for (int tries = 4096; digitalRead(m_pin) && tries > 0; tries--) ;
           // Delay to sample bit value
           delayMicroseconds(20);
           if (digitalRead(m_pin)) r |= bitMask;
           delayMicroseconds(80);
         }
 
         interrupts();
         return r;
       }
 
       bool read(void * buf, size_t count) {
         uint8_t * bp = (uint8_t * ) buf;
         do {
           uint8_t value = read();
           * bp++ = value;
         } while (--count);
 
         return true;
       }
 


       void write(uint8_t value, uint8_t bits = 8) {
         do {
           noInterrupts();
           pinMode(m_pin,INPUT);
           // Wait for bit start
           for (uint16_t tries = 4096; digitalRead(m_pin) && tries > 0; tries--) ;

           if ((value & 0x01) == 0) {
            pinMode(m_pin,OUTPUT);
            digitalWrite(m_pin,LOW);
             delayMicroseconds(33);
             pinMode(m_pin,INPUT);
            } else {
             delayMicroseconds(33);
           }
           interrupts();
           value >>= 1;

           // Wait for end
        //   pinMode(m_pin,INPUT);
           for (uint16_t tries = 4096; !digitalRead(m_pin) && tries > 0; tries--) ;
           

         } while (--bits);
       }
 
       void write(const void * buf, size_t count) {
         // Write bytes and calculate cyclic redundancy check-sum
         const uint8_t * bp = (const uint8_t * ) buf;
         do {
           delayMicroseconds(40);
           uint8_t value = * bp++;
           write(value);
         } while (--count);
       }

       //sets the overheat value
       void set_overheat(uint16_t value){
          uint8_t overheat_prec=value>75?75:value;//only goes up to 75 for older batteries
          uint8_t overheat_div5=overheat_prec/5;
          overheat_div5|=overheat_div5?0x20:0x0;
          m_rom[25]=SWAP_NIBBLES(overheat_div5); //old protocol puts it in rom here
          overheat=value;
       }


       //sets the overcurrent value
       void set_overcurrent(uint16_t value){
        uint8_t overcurrent_perc=value>80?80:value;//only goes up to 75
        uint8_t overcurrent_div=overcurrent_perc/5.33f;
        overcurrent_div= ~overcurrent_div;//somehow this parameter is an inverse
        overcurrent_div&=0xF;
        
        //bit 1 to 3 are overcurrent disable flags ( 0x0f )
        m_rom[24]=(overcurrent_div&0x0f)<<4 | (overcurrent_perc?1:3); //old protocol puts it in ROM here
        overcurrent=value;
       }

       void set_health(uint16_t value){
        health=value;
       }


       //sets the cycle count
       //for older batteries health = 100-(value/8.96)
       //health declines 1 bar per 224 cycles
       void set_cycle_count(uint16_t value){
        m_rom[26] =SWAP_NIBBLES(value>>8);
        m_rom[27] =SWAP_NIBBLES(value&0xff) ;
       }

       void set_extended(bool value){
        enable_extended=value;
       }

       void set_error(uint8_t value){
        m_rom[20]|=value&0x0f; 
       }


       //makita command process, please call in a continuous loop
       bool rom_command() {
         // Wait for reset
         byte buff[4];
 
         if (!reset()) return (false);

         int r = read();

         // Standard ROM commands
         if (r == 0x33) {

          
          uint8_t idbytes[] = { 0x16,0x07,0x13,0x64,0x14,0x0a,0x0e,0x69};
          write((void * ) idbytes, 8);

          if(read()!=0xF0) return;//only support this command for now
          read();
          write(m_rom, 32);
          return false;
        }

        if(enable_extended == false)return false;

         //extended protocol for newer batteries
         if (r == 0xCC) {
           r = read();
 
           if (r == 0xFF) { //no command recieved
             return false;
           }
 
           if (r == 0xDC) {
            uint8_t dcbytes[17] = { 0x1A, 0x01,0x0A,0x00,0x02,0x03,0x00,0x08,0x24,0x19,0x2A,0x3D,0x05,0x00,0x00,0x00,0x06};
             r = read();
             write(dcbytes, 17);
             return true;
           }
 
           if (r == 0xD4) {
             r = read();
 
             if (r == 0x50) {
               read(buff, 2);
               write(0x55); 
               write(10 + round(health/14) ); 
               write(0x06);
               return false;
             }
 
             if (r == 0x8D) {
              int overheat_div=round(overheat/2);

               read(buff, 2);
               write(0x00);
               write(0x00);
               write(0xFE);
               write(0x00);
               write(0x00); 
               write( (overheat_div&0xF) << 4 );
               write( (overheat_div&0xF0) >> 4 );
               write(0x06);
 
               return false;
             }
 
             if (r == 0xBA) {
              int overcurrent_div=round(overcurrent/2);
               read(buff, 2);
               write(overcurrent_div); //overcurrent 1 is 5%
               write(0x06);
             }
 
           }
 
           if (r == 0xD6) {
             r = read();
             read(buff, 1);
 
             if (r == 0x09) {
               write(0x00);
               write(0x00); //ff = 100, overcurrent 
               write(0x06);
               return true;
             }
 
             if (r == 0x38) {
               write(0x00);
               write(0x00);
               write(0xf0);
               write(0x06);
               return true;
             }
 
             if (r == 0x5B) {
               read(buff, 1);
               write(0x00); //overheat
               write(0x00); //overheat
               write(0x00); //overheat ( lsb -1 )
               write(0x00); //overheat ( lsb)
               write(0x06);
             }
 
             return false;
           }
 
           if (r == 0xD7) {
             r = read();
 
             if (r == 0x00) { //battery voltages
               read(buff, 2);
               uint8_t d7buf[] = {0xA2,0x4A,0xF1,0x0E,0xEA,0x0E,0xEC,0x0E,0xEC,0x0E,0xEE,0x0E,0x06};
               write(d7buf, sizeof(d7buf));
               return false;
             }
 
             if (r == 0x0E) {
               read(buff, 2);
               write(0x7e);
               write(0x09);
               write(0x06);
               return false;
             }
 
             if (r == 0x19) {
               read(buff, 2);
               write(0xd1);
               write(0xd5);
               write(0xA0);
               write(0x00);
               write(0x06);
             }
           }
 
           if (r == 0xD9) {
             write(0x96);
             write(0xA5);
             write(0x06);
           }
 
         }
 

 
         //ignore other commands
         return (false);
       }
 
 
       private:
       
       /** Reset detect timestamp. */
       uint32_t m_timestamp=0;
       bool enable_extended=false;
       uint16_t overheat=0;
       uint16_t overcurrent=0;
       uint16_t health=100;
       uint8_t m_rom[32];
      };
