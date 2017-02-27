
#ifndef __WORKFLOW_H
#define __WORKFLOW_H

#include "typealias.h"
#include "config.h"

typedef bool (*wf_cb) (char *, int);
typedef void (*wf_st) (void);
typedef void (*wf_cl) (void);
typedef void (*wf_to) (void);
typedef void (*wf_go) (u8);

typedef struct {
	u8 uart_id;
	u8 workflow_id;
	u8 task_id;
	u8 processing;
	wf_cb callback;
	wf_st start;
	wf_cl clean;
	wf_to timeout;
	wf_go go;
} workflow_t;

#define SIMPLE_WORKFLOW_TIMEOUT 1

typedef bool (*s_wf_cb) (void);
typedef void (*s_wf_go) (void);

typedef enum simple_workflow_status {
	empty = 0,
	wait_for_start,
	processing,
	wait_for_finish
} s_wf_status;

typedef struct {
	u8 uart_id;
	u8 processing;
	s_wf_status status;
	s_wf_go go;
	s_wf_cb callback;
} s_workflow_t;


#define DW_READ_WORKFLOW_ID	0

#define WORKFLOW_ID_TOTAL	1
#define TASK_ID_NONE					255

extern s_workflow_t xdata simple_workflow[MAX_UART_SUPPORT];
extern workflow_t xdata *current_workflow[MAX_UART_SUPPORT];
extern workflow_t xdata *workflow_registration[WORKFLOW_ID_TOTAL];
extern u8 xdata workflow_flow_duty_context;
extern u8 xdata workflow_finish_duty_context;

void workflow_init();

void start_workflow(workflow_t xdata *);
void start_simple_workflow(u8 uart_id, s_wf_go, s_wf_cb);

// 
void finish_workflow_before_next_flow(u8 wf_id);
void continue_workflow_at_next_round(u8 wf_id);

// TASK_DUTY handler
void wf_finish_duty_task();
void wf_flow_duty_task();
void wf_chk_timeout_duty_task();

//
void s_wf_duty_task();
void s_wf_chk_timeout_duty_task();

//
typedef enum response_check_result {
	failure = 0,
	success,
	ninterested = 99
} chk_res_t;

// 
bool workflow_check_response_full(char xdata *resp, char *expect);
bool workflow_check_response_prefix(char xdata *resp, char *expect_prefix);


#endif

