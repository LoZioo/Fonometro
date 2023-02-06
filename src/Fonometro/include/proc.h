// Setup routines.
inline void setup_GPIOs(), setup_wifi(), setup_timers(), setup_IPCs(), spawn_threads();

// ISR.
void IRAM_ATTR timer0_OVF_ISR(), timer1_OVF_ISR();

// Macros.
inline void start_timer(hw_timer_t*), stop_timer(hw_timer_t*);
