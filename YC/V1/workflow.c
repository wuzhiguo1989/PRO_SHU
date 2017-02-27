#include "workflow.h"
#include "task.h"
#include <stdio.h>
#include <string.h>

s_workflow_t xdata simple_workflow[MAX_UART_SUPPORT];
workflow_t xdata *current_workflow[MAX_UART_SUPPORT];
workflow_t xdata *workflow_registration[WORKFLOW_ID_TOTAL];

u8 xdata workflow_finish_duty_context = 0;
u8 xdata workflow_flow_duty_context = 0;

void workflow_init() {
	u8 i;
	for (i = 0; i < MAX_UART_SUPPORT; ++i) {
		current_workflow[i] = NULL;
	}

	for (i = 0; i< MAX_UART_SUPPORT; ++i) {
		simple_workflow[i].uart_id = i;
		simple_workflow[i].processing = 0;
		simple_workflow[i].status = empty;
		simple_workflow[i].go = NULL;
		simple_workflow[i].callback = NULL;
	}
}

void start_workflow(workflow_t xdata *wf) {
	if (wf == NULL) return;
	printf("workflow [%bu] on COM%bu\r\n", (u8) wf->workflow_id, (u8) wf->uart_id + 1);
	if (wf->uart_id >= MAX_UART_SUPPORT) return; 
	printf("Loading workflow [%bu] ... \r\n", (u8) wf->workflow_id);
	current_workflow[wf->uart_id] = wf;
	// clean context
	workflow_finish_duty_context &= ~(1<<wf->workflow_id);
	workflow_flow_duty_context &= ~(1<<wf->workflow_id);

	current_workflow[wf->uart_id]->start();
}

static void finish_workflow(workflow_t xdata *wf) {
	if (wf == NULL) return;
	if (wf->uart_id >= MAX_UART_SUPPORT) return; 
	printf("Finishing workflow [%bu] ... \r\n", (u8) wf->workflow_id);
	current_workflow[wf->uart_id] = NULL;
	wf->task_id = TASK_ID_NONE;
	wf->processing = 0;
	if (wf->clean) {
		wf->clean();
	}
}

void finish_workflow_before_next_flow(u8 wf_id) {
	workflow_flow_duty_context &= ~(1<<wf_id);
	workflow_finish_duty_context |= (1<<wf_id);
	call_for_duty(WF_FINISH_DUTY);
}

void continue_workflow_at_next_round(u8 wf_id) {
	workflow_flow_duty_context |= (1<<wf_id);
	call_for_duty(WF_FLOW_DUTY);
}


void start_simple_workflow(u8 uart_id, s_wf_go go, s_wf_cb callback) {
	if (uart_id < MAX_UART_SUPPORT) {
		s_workflow_t xdata *wf = &simple_workflow[uart_id];
		if (wf->status != empty) return;

		wf->status = wait_for_start;
		wf->processing = SIMPLE_WORKFLOW_TIMEOUT;
		wf->go = go;
		wf->callback = callback;
		call_for_duty(S_WF_DUTY);
	}
	
}

static bool workflow_check_timeout(workflow_t xdata *, bool auto_finish);
static bool simple_workflow_check_timeout(s_workflow_t xdata *);

void wf_finish_duty_task() {
	u8 i;
    for (i = 0; i < WORKFLOW_ID_TOTAL; ++i) {
    	if (workflow_finish_duty_context & (1<<i)) {
    		workflow_t xdata *wf = workflow_registration[i];
			finish_workflow(wf);
    		workflow_finish_duty_context &= ~(1<<i);
    	}
    }
}

void wf_flow_duty_task() {
	u8 i;
	for (i = 0; i < WORKFLOW_ID_TOTAL; ++i) {
    	if (workflow_flow_duty_context & (1<<i)) {
    		workflow_t xdata *wf = workflow_registration[i];
    		u8 task_id = wf->task_id;
    		wf->go(task_id);
    		workflow_flow_duty_context &= ~(1<<i);
    	}
    }
}

void wf_chk_timeout_duty_task() {
	u8 i;
	for (i = 0; i < WORKFLOW_ID_TOTAL; ++i) {
        workflow_check_timeout(workflow_registration[i], true);
    }
}

void s_wf_duty_task() {
	u8 i;
	s_workflow_t xdata *wf;
	for (i = 0; i < MAX_UART_SUPPORT; ++i) {

		wf = &simple_workflow[i];
		if (wf->status == empty) continue;
		if (wf->status == processing) continue;

		if (wf->status == wait_for_start) {
			if (wf->go) {
				wf->go();
				wf->status = processing;
				// no need to call_for_duty
			}
			else
				wf->status = wait_for_finish;
				// no need to call_for_duty
		}

		if (wf->status == wait_for_finish) {
			wf->status = empty;
			wf->processing = 0;
			wf->go = NULL;
			wf->callback = NULL;
		}
	}
}

void s_wf_chk_timeout_duty_task() {
	u8 i;
	for (i = 0; i < MAX_UART_SUPPORT; ++i) {
		simple_workflow_check_timeout(&simple_workflow[i]);
	}

}

static bool simple_workflow_check_timeout(s_workflow_t xdata *wf) {
	// 
	if (wf->status == empty || 
		wf->status == wait_for_start ||
		wf->status == wait_for_finish) 
		return false;

	// loading 
	if (wf->go == NULL || wf->callback == NULL) return false;
	// processing
	if (wf->status == processing && wf->processing && !--wf->processing) {
		wf->status = wait_for_finish;
		call_for_duty(S_WF_DUTY);
		return true;
	}

	return false;
}

static bool workflow_check_timeout(workflow_t xdata *wf, bool auto_finish) {
	// NULL pt check
	if (wf == NULL) return false;
	// wf check
	if (wf->uart_id >= MAX_UART_SUPPORT) return false;
	// No workflow processing at all
	if (current_workflow[wf->uart_id] == NULL) return false;
	// Current workflow is not what we checking
	if (current_workflow[wf->uart_id]->workflow_id != wf->workflow_id) return false;
	// process count down to zero 
	if (wf->processing && !--wf->processing) {
		printf("workflow [%bu] timeout\r\n", (u8) wf->workflow_id);
		if (auto_finish)
			finish_workflow_before_next_flow(wf->workflow_id);

		if (wf->timeout)
			wf->timeout();
		return true;
	}
	return false;
}

bool workflow_check_response_full(char xdata *resp, char *expect) {
	return strcmp(resp, expect) == 0;
}

bool workflow_check_response_prefix(char xdata *resp, char *expect_prefix) {
	int expect_prefix_size = strlen(expect_prefix);
	return strncmp(resp, expect_prefix, expect_prefix_size) == 0;
}