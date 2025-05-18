#include <alias.h>

template <> uint32_t sleep_controller::_startMs =0;
template <> volatile bool sleep_controller::_lpcompIntFired = false;

#if 1
extern "C" {
    //nrf_nvic_state_t nrf_nvic_state = {0}; // SoftDevice側で有効化してるようなので不要

    /**
     * @brief LPCOMP割り込みハンドラ.割り込みイベントは_lpcompIntFiredでチェック
     */ 
    void LPCOMP_IRQHandler(void)
    {
        sleep_controller::_lpcompIntFired = true;
        NRF_LPCOMP->EVENTS_CROSS = 0;
        DEBUG_PRINTF("on call LPCOMP_IRQHandler (sleep)");
        

        return ;
    }
}

#endif