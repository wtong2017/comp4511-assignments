#include <linux/kernel.h>
#include <linux/syscalls.h>
#include "sched.h"

#define WRR_TIMESLICE (100 * HZ / 1000)

/* Implementation of the weight round-robin scheduler */
void init_wrr_rq(struct wrr_rq *wrr_rq) {
	INIT_LIST_HEAD(&wrr_rq->queue);
}

static void enqueue_task_wrr(struct rq *rq, struct task_struct *p, int flags) {
	struct sched_wrr_entity *wrr_se = &p->wrr;
	list_add_tail(&wrr_se->run_list, &rq->wrr.queue);
	printk(KERN_INFO "[SCHED_WRR] ENQUEUE: Process-%d\n", p->pid);
}

static void dequeue_task_wrr(struct rq *rq, struct task_struct *p, int flags) {
	struct sched_wrr_entity *wrr_se = &p->wrr;
	list_del(&wrr_se->run_list);
	printk(KERN_INFO "[SCHED_WRR] DEQUEUE: Process-%d\n", p->pid);
}

static void yield_task_wrr(struct rq *rq) {
	struct sched_wrr_entity *wrr_se = &rq->curr->wrr;
	struct wrr_rq *wrr_rq = &rq->wrr;
	list_move_tail(&wrr_se->run_list, &wrr_rq->queue);
}

static void check_preempt_curr_wrr(struct rq *rq, struct task_struct *p, int flags) {
	if (rq->curr->policy == SCHED_FIFO || rq->curr->policy == SCHED_RR) {
		return;
	}
	if (rq->curr->policy == SCHED_WRR) {
		return;
	}
	resched_task(rq->curr);
}

static struct task_struct *pick_next_task_wrr(struct rq *rq) {
	struct sched_wrr_entity *wrr_se = NULL;
	struct task_struct *p = NULL;
	struct wrr_rq *wrr_rq = &rq->wrr;
	if (list_empty(&wrr_rq->queue)) {
		return NULL;
	}
	wrr_se = list_entry(wrr_rq->queue.next, struct sched_wrr_entity, run_list);
	p = container_of(wrr_se, struct task_struct, wrr);
	return p;
}

static void put_prev_task_wrr(struct rq *rq, struct task_struct *p) {
	/* it is the place to update the cureent task's runtime statistics */
}

static void set_curr_task_wrr(struct rq *rq) {

}

static void task_tick_wrr(struct rq *rq, struct task_struct *p, int queued) {
	if (p->policy != SCHED_WRR) {
		return;
	}
	
	printk(KERN_INFO "[SCHED_WRR] Task Tick: Process-%d with weight %d = %d\n", p->pid, p->wrr.wrr_weight, p->wrr.time_slice);
	
	if (--p->wrr.time_slice)
		return;
	p->wrr.time_slice = WRR_TIMESLICE * p->wrr.wrr_weight;
	list_move_tail(&p->wrr.run_list, &rq->wrr.queue);
	set_tsk_need_resched(p);
}

unsigned int get_rr_interval_wrr(struct rq *rq, struct task_struct *p) {
	return WRR_TIMESLICE * p->wrr.wrr_weight;
}

static void prio_changed_wrr(struct rq *rq, struct task_struct *p, int oldprio) {

}

static void switched_to_wrr(struct rq *rq, struct task_struct *p) {

}

const struct sched_class wrr_sched_class = {
	.next = &fair_sched_class,
	.enqueue_task = enqueue_task_wrr,
	.dequeue_task = dequeue_task_wrr,
	.yield_task = yield_task_wrr,
	.check_preempt_curr = check_preempt_curr_wrr,
	.pick_next_task = pick_next_task_wrr,
	.put_prev_task = put_prev_task_wrr,
	.set_curr_task = set_curr_task_wrr,
	.task_tick = task_tick_wrr,
	.get_rr_interval = get_rr_interval_wrr,
	.prio_changed = prio_changed_wrr,
	.switched_to = switched_to_wrr,
};

/* Implementation of set_wrr_weight syscall */
asmlinkage long sys_set_wrr_weight(pid_t pid, int weight) {
	struct task_struct *p = find_task_by_vpid(pid);
	printk(KERN_INFO "Weight: %i\n", weight);
	p->wrr.wrr_weight = weight;
	return 0;
}
