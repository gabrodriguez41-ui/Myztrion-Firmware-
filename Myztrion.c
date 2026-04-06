#include <stdio.h>
#include <string.h>
#include <hardware/adc.h>
#include <hardware/clocks.h>
#include <hardware/dma.h>
#include <hardware/irq.h>
#include <hardware/pwm.h>
#include <pico/binary_info.h>
#include <pico/multicore.h>
#include <pico/stdlib.h>
#include <pico/unique_id.h>

#include "Myztrion.h"
#include "include/stepper.h"

#include "include/identify.c"
#include "include/gpio.c"
#include "include/adc_builtin.c"
#include "include/pwm.c"
#include "include/stepper.c"


 
typedef struct { void (*command_func)(); void (*report_struct); } message_descriptor;
message_descriptor message_table[] = 
        {   
                {&identify,			&identify_report},  
                {&gpio_out,			&gpio_out_report},
                {&gpio_in,			&gpio_in_report},
                {&gpio_on_change,	&gpio_on_change_report},
                {&gpio_highz,		&gpio_highz_report},
                {&gpio_pull,		&gpio_pull_report},
                {&gpio_out_seq,		&gpio_out_seq_report},
                {&adc,				&adc_report},
                {&adc_stop,		    &adc_stop_report},
                {&pwm_configure_pair, &pwm_configure_pair_report},
                {&pwm_set_value,	&pwm_set_value_report},
                {&stepper_init,		&stepper_init_report},
                {&stepper_move,		&stepper_move_report},
                {&stepper_status,	&stepper_status_report},
                
			 
        };  

                

static inline void rx_next_command() {
    int packet_size;
    uint8_t packet_data;
    message_descriptor message_entry;
    command_buffer[0] = 0x00;

    
    if ((packet_size = getchar_timeout_us(0)) == PICO_ERROR_TIMEOUT) {
        return; 
    } else {
      
        for (int i = 0; i < packet_size; i++) {
            while ((packet_data = (uint8_t) getchar_timeout_us(0)) == PICO_ERROR_TIMEOUT) 
                busy_wait_us_32(1); 
            command_buffer[i] = packet_data;
        }

        
        if (command_buffer[0] >= ARRAY_LEN(message_table))  
            return; 

        
        message_entry = message_table[command_buffer[0]];
        message_entry.command_func(command_buffer);
    }
}

inline void tx_next_report() {
    fwrite(&txbuf[TXBUF_LEN*txbuf_tosend], txbuf_struct_len[txbuf_tosend], 1, stdout);
    if (txbuf_data_len[txbuf_tosend]) {
        fwrite(txbuf_data_ptr[txbuf_tosend], txbuf_data_len[txbuf_tosend], 1, stdout);
    }
    if (txbuf_data_write_lock_ptr[txbuf_tosend]) {
        *txbuf_data_write_lock_ptr[txbuf_tosend] = 0; 
    }
    fflush(stdout); 
}



void prepare_report(void* headerptr, uint16_t headersize, void* dataptr, 
		uint16_t datasize, uint8_t make_copy_of_data) {
    
    prepare_report_wrl(headerptr, headersize, dataptr, datasize, make_copy_of_data, 0x0000); 
}


void prepare_report_wrl(void* headerptr, uint16_t headersize, void* dataptr, 
		uint16_t datasize, uint8_t make_copy_of_data, uint8_t* data_write_lock_ptr) {
	while (txbuf_lock); txbuf_lock=1;
	memcpy(&txbuf[TXBUF_LEN*txbuf_tofill], headerptr, headersize);
	if (make_copy_of_data) { 
		
		txbuf_struct_len[txbuf_tofill] = headersize + datasize;
		txbuf_data_ptr[txbuf_tofill] = 0x0000;
		txbuf_data_len[txbuf_tofill] = 0;
		txbuf_data_write_lock_ptr[txbuf_tofill] = 0x0000;
		memcpy(&txbuf[TXBUF_LEN*txbuf_tofill+headersize], dataptr, datasize);
	} else {  
		txbuf_struct_len[txbuf_tofill] = headersize;
		txbuf_data_ptr[txbuf_tofill] = dataptr;
		txbuf_data_len[txbuf_tofill] = datasize;
		txbuf_data_write_lock_ptr[txbuf_tofill] = data_write_lock_ptr;
	}
	txbuf_tofill = (txbuf_tofill + 1) % TXBUF_COUNT;
	txbuf_lock=0;
    
}





volatile uint8_t timer10khz_triggered = 0;
bool timer10khz_update_routine(struct repeating_timer *t) {
	timer10khz_triggered = 1;  // small todo: is there some more elegant semaphore for this ?
	return true;
}


void core1_main() { 
	
    while (true) {

		if (timer10khz_triggered) {
			timer10khz_triggered = 0;
			stepper_update();
		}
    }
}

int main() {  
	
    bi_decl(bi_program_description("RP2 as universal platform for data acquisition and experiment automation"));
    bi_decl(bi_program_url("https://github.com/gabrodriguez41-ui/Myztrion"));
    bi_decl(bi_1pin_with_name(PICO_DEFAULT_LED_PIN, "Diagnostic LED, other pins assigned run-time"));

    set_sys_clock_khz(250000, false); 
	
    stdio_set_translate_crlf(&stdio_usb, false); 
    stdio_init_all();

	gpio_init(PICO_DEFAULT_LED_PIN); gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
	gpio_init(DEBUG_PIN); gpio_set_dir(DEBUG_PIN, GPIO_OUT); 
	gpio_init(DEBUG2_PIN); gpio_set_dir(DEBUG2_PIN, GPIO_OUT); 

	BLINK_LED_US(5000);
	busy_wait_us_32(100000); 
	BLINK_LED_US(5000);

    multicore_launch_core1(core1_main); 

    
    for (uint8_t report_code = 0; report_code < ARRAY_LEN(message_table); report_code++) {
		*((uint8_t*)(message_table[report_code].report_struct)) = report_code; 
    }


	
	
	struct repeating_timer timer;
	long usPeriod = -100;  
	add_repeating_timer_us(usPeriod, timer10khz_update_routine, NULL, &timer);

	iADC_DMA_init();

	while (true)  
	{ 
		rx_next_command();

		if (txbuf_tosend != txbuf_tofill) {
            tx_next_report();
			txbuf_tosend = (txbuf_tosend + 1) % TXBUF_COUNT;

		
		}

		iADC_on_buffer_transmitted();
	}
}




