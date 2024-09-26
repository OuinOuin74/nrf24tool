
// serial stuff
#include <stdio.h>
#include <stdlib.h>

#include "nrf24l01.h"

// frequency we're running at
#define	SYS_FREQ 64000000

//Taken from tft_master.h, by Syed Tahmid Mahbub
void nrf_delay_ms(unsigned long i){
/* Create a software delay about i ms long
 * Parameters:
 *      i:  equal to number of milliseconds for delay
 * Returns: Nothing
 * Note: Uses Core Timer. Core Timer is cleared at the initialiazion of
 *      this function. So, applications sensitive to the Core Timer are going
 *      to be affected
 */
    unsigned int j;
    j = dTime_ms * i;
    WriteCoreTimer(0);
    while (ReadCoreTimer() < j);
}

//Taken from tft_master.h, by Syed Tahmid Mahbub
void nrf_delay_us(unsigned long i){
/* Create a software delay about i us long
 * Parameters:
 *      i:  equal to number of microseconds for delay
 * Returns: Nothing
 * Note: Uses Core Timer. Core Timer is cleared at the initialiazion of
 *      this function. So, applications sensitive to the Core Timer are going
 *      to be affected
 */
    unsigned int j;
    j = dTime_us * i;
    WriteCoreTimer(0);
    while (ReadCoreTimer() < j);
}

char rf_spiwrite(unsigned char c){ // Transfer to SPI
    while (TxBufFullSPI2());
    WriteSPI2(c);
    while( !SPI2STATbits.SPIRBF); // check for complete transmit
    return ReadSPI2();
}

void init_SPI(){
    // Set up SPI2 to be active high, master, 8 bit mode, and ~4 Mhz CLK
    SpiChnOpen(2, SPI_OPEN_MSTEN | SPI_OPEN_MODE8 | SPI_OPEN_ON | SPI_OPEN_CKE_REV, 16);
    // Set SDI2 to pin 12
    PPSInput(3, SDI2, RPA4);
    // Set SDO2 to pin 9
    PPSOutput(3, RPA2, SDO2);
}

void nrf_setup(){
    nrf_delay_ms(200);
    init_SPI();   
    // Set external interrupt 1 to pin 21
    PPSInput(4, INT1, RPB10);
    
    ConfigINT1(EXT_INT_PRI_2 | FALLING_EDGE_INT | EXT_INT_ENABLE);
    EnableINT1;
    mINT1ClearIntFlag();
    
    TRIS_csn = 0;
    TRIS_ce = 0;
    
    nrf_reset();
}

// Read a register from the nrf24l01
// reg is the array to read, len is the length of data expected to be received (1-5 bytes)
// NOTE: only address 0 and 1 registers use 5 bytes all others use 1 byte 
// NOTE: writing or reading payload is done using a specific command
void nrf_read_reg(char reg, char * buff, int len){
    //char reg_read[5]; // register used for reading a single register
    int i = 0;
    _csn = 0; // begin transmission
    status = rf_spiwrite(nrf24l01_R_REGISTER | reg); // send command to read register
    
    for(i=0;i<len;i++){
        buff[i] = rf_spiwrite(nrf24l01_SEND_CLOCK); // send clock pulse to continue receiving data
    }
    _csn = 1; // end transmission
}


void nrf_write_reg(char reg, char * data, char len){
    int i = 0;
    _csn = 0; // begin transmission
    status = rf_spiwrite(nrf24l01_W_REGISTER | reg); // send command to write reg
    for(i=0;i<len;i++){
        rf_spiwrite(data[i]); // write each char/byte to address reg
    }
    _csn = 1; // end transmission
}

// flushes the tx FIFO
void nrf_flush_tx(){
    _csn = 0;
    rf_spiwrite(nrf24l01_FLUSH_TX);
    _csn = 1;
    
}

// flushes the rx FIFO
// NOTE: do not use while sending acknowledgement
void nrf_flush_rx(){
    _csn = 0;
    rf_spiwrite(nrf24l01_FLUSH_RX);
    _csn = 1;   
}


int nrf_empty_tx_fifo(){
    char reg;
    nrf_read_reg(nrf24l01_FIFO_STATUS, &reg, 1);
    char empty = reg & nrf24l01_FIFO_STATUS_TX_EMPTY;
    if(empty == nrf24l01_FIFO_STATUS_TX_EMPTY){
        return 1;
    }else{
        return 0;
    }
}

int nrf_full_tx_fifo(){
    char reg;
    nrf_read_reg(nrf24l01_FIFO_STATUS, &reg, 1);
    char full = reg & nrf24l01_FIFO_STATUS_TX_FULL;
    if(full == nrf24l01_FIFO_STATUS_TX_FULL){
        return 1;
    }else{
        return 0;
    }
}

// This is meant for library use
int nrf_get_payload_width(){
    char width;
    _csn = 0;
    status = rf_spiwrite(nrf24l01_R_RX_PL_WID);
    width = rf_spiwrite(nrf24l01_SEND_CLOCK);
    _csn = 1;
    return width;
}

// Write a payload to be sent over the radio
// data: array of chars to be sent (1-32 chars/bytes)
// len: amount of chars in array/bytes to be sent
void nrf_write_payload(char * data, char len){
    int i = 0;
    //Write packet to TX FIFO before pulsing
    _csn = 0; // begin transmission
    status = rf_spiwrite(nrf24l01_W_TX_PAYLOAD); // send the command to write the payload
    for(i=0;i<len;i++){
        rf_spiwrite(data[i]); // write each char/byte to tx payload one at a time
    }
    _csn = 1; // end transmission
    
}

// should read the payload into a buffer
void nrf_read_payload(char * buff){
    char dpl;   
    // get the pipe the payload was received on
    nrf_read_reg(nrf24l01_STATUS, &status, 1);
    pipe_no = (status & 0x0E) >> 1;
    
    nrf_read_reg(nrf24l01_FEATURE, &dpl, 1);
    // check if dynamic payload lengths are enabled
    if(dpl & 0x04){
        width = nrf_get_payload_width();
        _csn = 0; // begin transmission
        status = rf_spiwrite(nrf24l01_R_RX_PAYLOAD); // send command to read payload
        int i;
        for(i=0;i<width;i++){
            buff[i] = rf_spiwrite(nrf24l01_SEND_CLOCK);
        }
        _csn = 1; // end transmission
    }else{
        _csn = 0; // begin transmission
        status = rf_spiwrite(nrf24l01_R_RX_PAYLOAD); // send command to read payload
        int i;
        for(i=0;i<payload_size;i++){
            buff[i] = rf_spiwrite(nrf24l01_SEND_CLOCK);
        }
        _csn = 1; // end transmission
    }

}

int nrf_get_payload(char * buff, char len){  
    //A payload is available
    if(received){
        int i;
        for(i=0;i<len;i++){
            buff[i]=RX_payload[i];
        }
        received = 0;
        return 1;
    }else{//No payload is available
        return 0;
    }
}

int nrf_payload_available(){
    return received;
}

int nrf_get_pipe(){
    return pipe_no;
}

// This is meant for user use
int nrf_get_width(){
    return width;
}

//Sets the power up bit and waits for the startup time, putting the radio in Standby-I mode
void nrf_pwrup(){
    char config;
    nrf_read_reg(nrf24l01_CONFIG, &config, 1);
    config |= nrf24l01_CONFIG_PWR_UP;
    nrf_write_reg(nrf24l01_CONFIG, &config, 1);
    nrf_delay_ms(2);//Delay for power up time
}

//Clear the pwr_up bit, transitioning to power down mode
void nrf_pwrdown(){
    char config;
    nrf_read_reg(nrf24l01_CONFIG, &config, 1);
    config &= ~(nrf24l01_CONFIG_PWR_UP);
    nrf_write_reg(nrf24l01_CONFIG, &config, 1);
    _ce = 0;
}

void nrf_state_pwr_down(){
    switch(state){
        case PWR_DOWN :
            break;
        case STANDBY_1 :
            nrf_pwrdown();
            break;
        case RX_MODE :
            _ce = 0;
            nrf_pwrdown();
            break;
        default :
            _ce = 0;
            nrf_pwrdown();      
    }
    state = PWR_DOWN;
}

void nrf_state_standby_1(){
    switch(state){
        case PWR_DOWN :
            nrf_pwrup();
            break;
        case STANDBY_1 :
            break;
        case RX_MODE :
            _ce = 0;
            break;
        default :
            _ce = 0;
            nrf_pwrup();      
    }
    state = STANDBY_1;
}

void nrf_state_rx_mode(){
    switch(state){
        case PWR_DOWN :
            nrf_state_standby_1();
            nrf_set_prim_rx();
            _ce = 1;
            nrf_delay_us(130);
            break;
        case STANDBY_1 :
            nrf_set_prim_rx();
            _ce = 1;
            nrf_delay_us(130);
            break;
        case RX_MODE :
            break;
        default :
            nrf_state_standby_1();
            nrf_set_prim_rx();
            _ce = 1;
            nrf_delay_us(130);     
    }
    state = RX_MODE;
}

void nrf_set_prim_rx(){
    char config;
    nrf_read_reg(nrf24l01_CONFIG, &config, 1);
    config |= nrf24l01_CONFIG_PRIM_RX;
    nrf_write_reg(nrf24l01_CONFIG, &config, 1);
}

void nrf_clear_prim_rx(){
    char config;
    nrf_read_reg(nrf24l01_CONFIG, &config, 1);
    config &= ~(nrf24l01_CONFIG_PRIM_RX);
    nrf_write_reg(nrf24l01_CONFIG, &config, 1);
}

// sets power of transmitter, possible values and definitions for them are
//  0dBm: nrf24l01_RF_SETUP_RF_PWR_0
// -6dBm: nrf24l01_RF_SETUP_RF_PWR_6
// -12dBm: nrf24l01_RF_SETUP_RF_PWR_12
// -18dBm: nrf24l01_RF_SETUP_RF_PWR_18
void nrf_set_transmit_pwr(char power){
    char setup;
    nrf_read_reg(nrf24l01_RF_SETUP, &setup, 1); // check value of setup register
    setup &= ~(nrf24l01_RF_SETUP_RF_PWR); // clear power bits in register
    setup |= power; // set power bits in register
    nrf_write_reg(nrf24l01_RF_SETUP, &setup, 1);  
}

// sets the rf data rate, possible values and definitoins for them are
// 250 kbps: nrf24l01_DR_LOW
// 1 Mbps: nrf24l01_DR_MED
// 2 Mbps: nrf24l01_DR_HIGH
void nrf_set_transmit_rate(char rate){
    char setup; // check value of setup register
    nrf_read_reg(nrf24l01_RF_SETUP, &setup, 1);
    setup &= 0xd7;  // clear data rate bits in register
    setup |= rate; // set data rate bits in register
    nrf_write_reg(nrf24l01_RF_SETUP, &setup, 1); 
}

void nrf_set_ard(char ard){
    char setup; // check value of setup register
    nrf_read_reg(nrf24l01_SETUP_RETR, &setup, 1);
    setup &= 0x0F;  // clear data rate bits in register
    setup |= ard; // set data rate bits in register
    nrf_write_reg(nrf24l01_SETUP_RETR, &setup, 1); 
}

void nrf_set_arc(char arc){
    char setup; // check value of setup register
    nrf_read_reg(nrf24l01_SETUP_RETR, &setup, 1);
    setup &= 0xF0;  // clear data rate bits in register
    setup |= arc; // set data rate bits in register
    nrf_write_reg(nrf24l01_SETUP_RETR, &setup, 1); 
}

void nrf_set_rf_ch(char ch){
    nrf_write_reg(nrf24l01_RF_CH, &ch, 1);
}

char nrf_received_pipe_num(){
    nrf_read_reg(nrf24l01_STATUS, &status, 1);
    return status & 0x0E;
}

void nrf_set_address_width(char width){
    char setting = width - 2;
    nrf_write_reg(nrf24l01_SETUP_AW, &setting, 1);
}

char * parse_addr(uint64_t address){
    static char addr[5];
    int i = 0;
    for(i = 0; i < 5;i++){
        addr[i] = (char)((address >> i*8) & 0xFF);
    }
    return &addr;
}

int nrf_set_rx_addr(int pipe, uint64_t address, int len){
    char width;
    nrf_read_reg(nrf24l01_SETUP_AW, &width, 1);
    width = width + 2;
    char * addr = parse_addr(address);
    
    if(pipe == 0 || pipe == 1){
        if(len != width){//Must equal preset address width
            return 0;
        }
        
    }else{
        if(len != 1){
            return 0;//For pipes 2,3,4,5 can only set last byte
        }
    }
    
    nrf_write_reg(nrf24l01_RX_ADDR_P0+pipe, addr, len);
    return 1;
}

void nrf_set_tx_addr(uint64_t address){
    char * addr = parse_addr(address);
    nrf_write_reg(nrf24l01_TX_ADDR, addr, 5);
}

void nrf_start_cont_wave(char pwr){
    nrf_state_standby_1();
    char setting;
    nrf_read_reg(nrf24l01_RF_SETUP, &setting, 1);
    setting |= nrf24l01_RF_SETUP_PLL_LOCK | nrf24l01_RD_SETUP_CONT_WAVE;
    nrf_write_reg(nrf24l01_RF_SETUP, &setting, 1);
    nrf_set_transmit_pwr(pwr);
    _ce = 1;   
}

void nrf_stop_cont_wave(){
    _ce = 0;
    char reg;
    nrf_read_reg(nrf24l01_RF_SETUP, &reg, 1);
    reg &= ~(nrf24l01_RF_SETUP_PLL_LOCK | nrf24l01_RD_SETUP_CONT_WAVE);  
    nrf_write_reg(nrf24l01_RF_SETUP, &reg, 1);
    nrf_pwrdown();
}

char nrf_received_pwr(){
    char pwr;
    nrf_read_reg(nrf24l01_RPD, &pwr, 1);
    pwr &= 0x01;
    return pwr;
}

void nrf_en_aa(int pipe){
    char num = 0x01;
    num = num << pipe;
    char reg;
    nrf_read_reg(nrf24l01_EN_AA, &reg, 1);
    reg |= num;
    // enable the pipe
    nrf_en_rxaddr(pipe);
    nrf_write_reg(nrf24l01_EN_AA, &reg, 1);
}

void nrf_dis_aa(int pipe){
    char num = 0x01;
    num = num << pipe;
    char reg;
    nrf_read_reg(nrf24l01_EN_AA, &reg, 1);
    num = ~num;
    reg &= num;
    nrf_write_reg(nrf24l01_EN_AA, &reg, 1);
}

void nrf_en_rxaddr(int pipe){
    char num = 0x01;
    num = num << pipe;
    char reg;
    nrf_read_reg(nrf24l01_EN_RXADDR, &reg, 1);
    reg |= num;
    nrf_write_reg(nrf24l01_EN_RXADDR, &reg, 1);
}

void nrf_dis_rxaddr(int pipe){
    char num = 0x01;
    num = num << pipe;
    char reg;
    nrf_read_reg(nrf24l01_EN_RXADDR, &reg, 1);
    num = ~num;
    reg &= num;
    nrf_write_reg(nrf24l01_EN_RXADDR, &reg, 1);
}

void nrf_set_pw(char width, int pipe){
   payload_size = width;
   nrf_write_reg(nrf24l01_RX_PW_P0 + pipe, &width, 1);
}

void nrf_en_dpl(int pipe){
    char num = 0x01;
    num = num << pipe;
    char reg;
    nrf_read_reg(nrf24l01_DYNPD, &reg, 1);
    // set bit corresponding to pipe
    reg |= num;
    nrf_write_reg(nrf24l01_DYNPD, &reg, 1);
    // set EN_DPL in FEATURE register
    nrf_read_reg(nrf24l01_FEATURE, &reg, 1);
    reg |= 0x04;
    nrf_write_reg(nrf24l01_FEATURE, &reg, 1);
    // enable autoack for the pipe
    nrf_en_aa(pipe);
}

void nrf_dis_dpl(int pipe){
    char num = 0x01;
    num = num << pipe;
    char reg;
    nrf_read_reg(nrf24l01_DYNPD, &reg, 1);
    // clear bit corresponding to pipe
    num = ~num;
    reg &= num;
    nrf_write_reg(nrf24l01_DYNPD, &reg, 1);
    if(reg == 0x00){ // check if dpl has been disabled in all pipes
        // clear EN_DPL in FEATURE register
        nrf_read_reg(nrf24l01_FEATURE, &reg, 1);
        reg &= ~0x04;
        nrf_write_reg(nrf24l01_FEATURE, &reg, 1);
    }
}

void nrf_en_dyn_ack(){
    char reg;
    nrf_read_reg(nrf24l01_FEATURE, &reg, 1);
    reg |= 0x01;
    nrf_write_reg(nrf24l01_FEATURE, &reg, 1);
}

void nrf_dis_dyn_ack(){
    char reg;
    nrf_read_reg(nrf24l01_FEATURE, &reg, 1);
    reg &= ~0x01;
    nrf_write_reg(nrf24l01_FEATURE, &reg, 1);
}

void nrf_en_ack_pay(){
    char reg;
    nrf_read_reg(nrf24l01_FEATURE, &reg, 1);
    reg |= 0x02;
    nrf_write_reg(nrf24l01_FEATURE, &reg, 1);
}

void nrf_dis_ack_pay(){
    char reg;
    nrf_read_reg(nrf24l01_FEATURE, &reg, 1);
    reg &= ~0x02;
    nrf_write_reg(nrf24l01_FEATURE, &reg, 1);
}

//Send a payload, with no checking for whether it was successfully received or not
//Does not provide reliable data transfer, but makes it possible to push out more packets
//and to have greater application level control over packet timing.
int nrf_send_payload_nonblock(char * data, char len){
    nrf_flush_tx();
    nrf_write_payload(data, len);//Send payload to FIFO
    nrf_state_standby_1();//Transition to standby 1 state to operate in a known state
    nrf_clear_prim_rx();
    
    _ce = 1;//Pulse the line to begin the transition to TX
    nrf_delay_us(100);
    _ce = 0;    
    
    nrf_delay_us(130);//RX Settling Time
 
    if(sent) return 1;
    else return 0;
}

// send a payload with auto ack. Returns 1 if packet was received correctly
int nrf_send_payload(char * data, char len){
    nrf_flush_tx();
    nrf_write_payload(data,len);
    nrf_state_standby_1();
    nrf_clear_prim_rx();
    _ce = 1;
    while(!sent && !error);
    _ce = 0;
    if(sent){
        sent = 0;
        return 1;
    }
    error = 0;
    return 0;
}


void nrf_reset(){
    char reg = 0x08;
    nrf_write_reg(nrf24l01_CONFIG, &reg, 1);
    reg = 0x3F;
    nrf_write_reg(nrf24l01_EN_AA, &reg, 1);
    reg = 0x03;
    nrf_write_reg(nrf24l01_SETUP_AW, &reg, 1);
    reg = 0x03;
    nrf_write_reg(nrf24l01_EN_RXADDR, &reg, 1);
    reg = 0x03;
    nrf_write_reg(nrf24l01_SETUP_RETR, &reg, 1);
    reg = 0x02;
    nrf_write_reg(nrf24l01_RF_CH, &reg, 1);
    reg = 0x0E;
    nrf_write_reg(nrf24l01_RF_SETUP, &reg, 1);
    reg = 0x70;
    nrf_write_reg(nrf24l01_STATUS, &reg, 1);
    reg = 0x00;
    nrf_write_reg(nrf24l01_OBSERVE_TX, &reg, 1);
    nrf_set_rx_addr(0, 0xE7E7E7E7E7, 5);
    nrf_set_rx_addr(1, 0xC2C2C2C2C2, 5);
    nrf_set_rx_addr(2, 0xC3, 1);
    nrf_set_rx_addr(3, 0xC4, 1);
    nrf_set_rx_addr(4, 0xC5, 1);
    nrf_set_rx_addr(5, 0xC6, 1);
    nrf_set_tx_addr(0xE7E7E7E7E7);
    nrf_set_pw(0, 0);
    nrf_set_pw(0, 1);
    nrf_set_pw(0, 2);
    nrf_set_pw(0, 3);
    nrf_set_pw(0, 4);
    nrf_set_pw(0, 5);
    reg = 0x00;
    nrf_write_reg(nrf24l01_DYNPD, &reg, 1);
    reg = 0x00;
    nrf_write_reg(nrf24l01_FEATURE, &reg, 1);
    nrf_flush_tx();
    nrf_flush_rx();
    state = PWR_DOWN;
}

void __ISR(_EXTERNAL_1_VECTOR, ipl2) INT1Handler(void){
    nrf_read_reg(nrf24l01_STATUS, &status, 1); // read the status register
    // check which type of interrupt occurred
    if (status & nrf24l01_STATUS_RX_DR) { // if data received
        nrf_read_payload(&RX_payload);
        received = 1; // signal main code that payload was received
        status |= nrf24l01_STATUS_RX_DR; // clear interrupt on radio
    }
        // if data sent or if acknowledge received when auto ack enabled
    else if (status & nrf24l01_STATUS_TX_DS) {    
        sent = 1; // signal main code that payload was sent
        status |= nrf24l01_STATUS_TX_DS; // clear interrupt on radio
    } else { // maximum number of retransmit attempts occurred
        error = 1; // signal main code that the payload was not received
        status |= nrf24l01_STATUS_MAX_RT; // clear interrupt on radio
    }
    
    nrf_write_reg(nrf24l01_STATUS, &status, 1);
    mINT1ClearIntFlag();
}
