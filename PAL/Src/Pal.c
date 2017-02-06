#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include "pal.h"
#include <time.h>
#include <sys/time.h>
#include <string.h> 
#include <signal.h>

/* === Externals ============================================================ */
extern At86rf215_Dev_t at86rf215_dev;
struct timeval start;


retval_t pal_init(void){
	if(-1==spi_init(at86rf215_dev.spi)){
		return FAILURE;
	}
	if(-1==gpio_init(at86rf215_dev.gpio_irq)){
		return FAILURE;
	}
	if(-1==gpio_init(at86rf215_dev.gpio_rest)){
		return FAILURE;
	}
	gettimeofday(&start,NULL);
	return MAC_SUCCESS;
}

void pal_timer_delay(uint16_t delay){//us
	struct timeval tv;
	tv.tv_sec=delay/1000000;
	tv.tv_usec=delay-tv.tv_sec*1000000;	
	select(0,NULL,NULL,NULL,&tv);

}

void TRX_RST_HIGH(){
	set_gpio_value(at86rf215_dev.gpio_rest,high);
}

void TRX_RST_LOW(){
	set_gpio_value(at86rf215_dev.gpio_rest,low);
}

gpio_value_t TRX_IRQ_GET(){
	return get_gpio_value(at86rf215_dev.gpio_irq);
}


void pal_get_current_time(uint32_t *current_time){
	struct  timeval  cur;
	gettimeofday(&cur,NULL);
	*current_time=(uint32_t)(1000000*(cur.tv_sec-start.tv_sec))+(cur.tv_usec-start.tv_usec);//us

}


retval_t pal_timer_start(uint16_t id,
						 timer_instance_id_t timer_instance_id,
						 uint32_t timer_count,
						 timeout_type_t timeout_type,
						 FUNC_PTR(timer_cb),
						 void *param_cb){
	timer_t timerid;
	struct sigevent evp;
	memset(&evp, 0, sizeof(struct sigevent));		//清零初始化
	evp.sigev_value.sival_int=timer_instance_id;
	evp.sigev_notify = SIGEV_THREAD;			//线程通知的方式，派驻新线程
	evp.sigev_notify_function = timer_cb;		//线程函数地址	
	if (timer_create(CLOCK_REALTIME, &evp, &timerid) == -1)
	{
		perror("fail to timer_create");
		return FAILURE;
	}
	struct itimerspec it;
	it.it_interval.tv_sec = 0;
	it.it_interval.tv_nsec = 0;
	it.it_value.tv_sec = timer_count/1000000;//s
	it.it_value.tv_nsec = timer_count*1000-it.it_value.tv_sec*1000000;//ns
	if (timer_settime(timerid, 0, &it, NULL) == -1)
	{
		perror("fail to timer_settime");
		return FAILURE;
	}
	return MAC_SUCCESS;

}

retval_t pal_timer_stop(uint16_t timer_id,
							timer_instance_id_t timer_instance_id){
	
	return MAC_SUCCESS;
}





