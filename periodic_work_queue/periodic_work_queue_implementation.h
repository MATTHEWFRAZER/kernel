#pragma once

void pwq_work_handler(struct work_struct *work);
int pwq_work_queue_init(void);
int pwq_work_queue_exit(void);
int pwq_set_delay(int delay);
